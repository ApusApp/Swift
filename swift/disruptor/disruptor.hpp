/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SWIFT_DISRUPTOR_DISRUPTOR_HPP__
#define __SWIFT_DISRUPTOR_DISRUPTOR_HPP__

#include <iostream>
#include <memory>
#include <vector>
#include <atomic>
#include <limits>
#include <unistd.h>
#include <assert.h>

namespace swift {
namespace disruptor {
    const int64_t kMaxInt64Value = std::numeric_limits<int64_t>::max ();
    class Eof : public std::exception
    {
    public:
        virtual const char* what () const noexcept { return "EOF"; }
    };

    /**
     *  A sequence number must be padded to prevent false sharing and
     *  access to the sequence number must be protected by memory barriers.
     *
     *  In addition to tracking the sequence number, additional state associated
     *  with the sequence number is also made available.  No false sharing
     *  should occur because all 'state' is only written by one thread. This
     *  extra state includes whether or not this sequence number is 'EOF' and
     *  whether or not any alerts have been published.
     */
    class Sequence
    {
    public:
        Sequence (int64_t value = 0)
            : sequence_ (value)
            , alert_ (0)
        {
        }

        int64_t aquire () const 
        { 
            return sequence_.load (std::memory_order_acquire); 
        }

        void store (int64_t value) 
        { 
            sequence_.store (value, std::memory_order_release); 
        }

        void set_eof ()
        {
            alert_ = 1;
        }

        void set_alert ()
        {
            alert_ = -1;
        }

        bool eof () const
        {
            return alert_ == 1;
        }

        bool alert () const
        {
            return alert_ != 0;
        }

        int64_t atomic_increment_and_get (uint64_t increment)
        {
            return sequence_.fetch_add (increment, std::memory_order::memory_order_release) + increment;
        }

    private:
        // on x86 cacheline is 64 byte size
        std::atomic<int64_t> sequence_;
        volatile int64_t     alert_;
        int64_t              padding_[6];
    };

    class EventCursor;

    /**
     *   A barrier will block until all cursors it is following are
     *   have moved past a given position. The barrier uses a
     *   progressive backoff strategy of busy waiting for 1000
     *   tries, yielding for 1000 tries, and the usleeping in 10 ms
     *   intervals.
     *
     *   No wait conditions or locks are used because they would
     *   be 'intrusive' to publishers which must check to see whether
     *   or not they must 'notify'. The progressive backoff approach
     *   uses little CPU and is a good compromise for most use cases.
     */
    class Barrier
    {
    public:
        void follows (std::shared_ptr<const EventCursor> event_cursor);

        /**
         *  Used to check how much you can read/write without blocking.
         *  @return the min position of every cusror this barrier follows.
         */
        int64_t get_min ();

        /**
         *  This method will wait until all s in seq >= pos using a progressive
         *  backoff of busy wait, yield, and usleep(10*1000)
         *  @return the minimum value of every dependency
         */
        int64_t wait_for (int64_t pos) const;

    private:
        mutable int64_t                                   last_min_;
        std::vector<std::shared_ptr<const EventCursor> >  limit_seq_;
    };

    /**
     *  Provides a automatic index into a ringbuffer with a power of 2 size.
     */
    template<typename EventType, uint64_t Size = 1024>
    class RingBuffer
    {
    public:
        typedef EventType event_type;

        static_assert (((Size != 0) && ((Size & (~Size + 1)) == Size)), "Ring buffer's size must be a power of 2");

        /** @return a read-only reference to the event at pos */
        const EventType& at (int64_t pos) const
        {
            return buffer_[pos & (Size - 1)];
        }

        /** @return a reference to the event at pos */
        EventType& at (int64_t pos)
        {
            return buffer_[pos & (Size - 1)];
        }

        /** useful to check for contiguous ranges when EventType is
         *  POD and memcpy can be used.  OR if the buffer is being used
         *  by a socket dumping raw bytes in.  In which case memcpy
         *  would have to use to ranges instead of 1.
         */
        int64_t get_buffer_index (int64_t pos) const
        {
            return pos & (Size - 1);
        }

        int64_t get_buffer_size () const
        {
            return Size;
        }

    private:
        EventType buffer_[Size];
    };

    /**
     *  A cursor is used to track the location of a publisher / subscriber within
     *  the ring buffer.  Cursors track a range of entries that are waiting
     *  to be processed.  After a cursor is 'done' with an entry it can publish
     *  that fact.
     *
     *  There are two types of cursors, read_cursors and write cursors. read_cursors
     *  block when they need to.
     *
     *  Events between [begin,end) may be processed at will for readers. When a reader
     *  is done they can 'publish' their progress which will move begin up to
     *  published position+1. When begin == end, the cursor must call wait_for(end),
     *  wait_for() will return a new 'end'.
     *
     *  @section read_cursor_example Read Cursor Example
     *  @code
     auto source   = std::make_shared<RingBuffer<EventType,SIZE>>();
     auto dest     = std::make_shared<RingBuffer<EventType,SIZE>>();
     auto p        = std::make_shared<WriteCursor>("write",SIZE);
     auto a        = std::make_shared<ReadCursor>("a");

     a->follows (p);
     p->follows (a);

     auto pos = a->begin ();
     auto end = a->end();
     while (true) {
        if (pos == end) {
            a->publish (pos - 1);
            end = a->wait_for (end);
        }

        dest->at (pos) = source->at (pos);
        ++pos;
     }
     *  @endcode
     *
     *  @section write_cursor_example Write Cursor Example
     *
     *  The following code would run in the publisher thread. The
     *  publisher can write data without 'waiting' until it pos is
     *  greater than or equal to end.  The 'initial condition' of
     *  a publisher is with pos > end because the write cursor
     *  cannot 'be valid' for readers until after the first element
     *  is written.
     *
     @code
     auto pos = p->begin ();
     auto end = p->end ();
     while (!done) {
        if (pos >= end) {
            end = p->wait_for (end);
        }

        source->at (pos) = i;
        p->publish (pos);
        ++pos;
     }
     // set eof to signal any followers to stop waiting after
     // they hit this position.
     p->set_eof();
     @endcode
     *
     */
    class EventCursor
    {
    public:
        EventCursor (int64_t pos = -1)
            : name_ ("")
            , begin_ (pos)
            , end_ (pos)
        {
        }

        EventCursor (const char* name, int64_t pos = 0)
            : name_ (name)
            , begin_ (pos)
            , end_ (pos)
        {
        }

        /** this event processor will process every event upto, but not including s */
        template<typename T>
        void follows (T&& s)
        {
            barrier_.follows (std::forward<T> (s));
        }

        /** returns one after cursor */
        int64_t begin () const
        {
            return begin_;
        }

        /** returns one after the last ready as of last call to wait_for () */
        int64_t end () const
        {
            return end_;
        }

        /** makes the event at pos available to those following this cursor */
        void publish (int64_t pos)
        {
            check_alert ();
            begin_ = pos + 1;
            cursor_.store (pos);
        }

        /** when the cusor hits the end of a stream, it can set the eof flag */
        void set_eof ()
        {
            cursor_.set_eof ();
        }

        /** If an error occurs while processing data the cursor can set an
         *  alert that will be thrown whenever another cursor attempts to wait
         *  on this cursor.
         */
        void set_alert (std::exception_ptr exception)
        {
            alert_ = std::move (exception);
            cursor_.set_alert ();
        }

        /** @return any alert set on this cursor */
        const std::exception_ptr& alert () const
        {
            return alert_;
        }

        /** If an alert has been set, throw! */
        inline void check_alert () const;

        /** the last sequence number this processor has completed.*/
        const Sequence& pos () const
        {
            return cursor_;
        }

        /** used for debug messages */
        const char* name () const
        {
            return name_;
        }

    protected:
        /** last know available, min(limit_seq_) */
        const char*                   name_;
        int64_t                       begin_;
        int64_t                       end_;
        std::exception_ptr            alert_;
        Barrier                       barrier_;
        Sequence                      cursor_;
    };

    /**
     *  Tracks the read position in a buffer
     */
    class ReadCursor : public EventCursor
    {
    public:
        ReadCursor (int64_t pos = 0) : EventCursor (pos) {}
        ReadCursor (const char* name, int64_t pos = 0) : EventCursor (name, pos) {}

        /** @return end() which is > pos */
        int64_t wait_for (int64_t pos)
        {
            try {
                return end_ = barrier_.wait_for (pos) + 1;
            }
            catch (const Eof&) {
                cursor_.set_eof ();
                throw;
            }
            catch (...) {
                set_alert (std::current_exception ());
                throw;
            }
        }

        /** find the current end without blocking */
        int64_t check_end ()
        {
            return end_ = barrier_.get_min () + 1;
        }
    };
    typedef std::shared_ptr<ReadCursor> read_cursor_ptr;

    /**
     *  Tracks the write position in a buffer.
     *
     *  Write cursors need to know the size of the buffer
     *  in order to know how much space is available.
     */
    class WriteCursor : public EventCursor
    {
    public:
        /** @param size - the size of the ringbuffer, required to do proper wrap detection */
        WriteCursor (int64_t size)
            : size_ (size)
            , size_m1_ (size - 1)
        {
            begin_ = 0;
            end_ = size_;
            cursor_.store (-1);
        }

        /**
         * @param name - name of the cursor for debug purposes
         * @param size - the size of the buffer.
         */
        WriteCursor (const char* name, int64_t size)
            : EventCursor (name)
            , size_ (size)
            , size_m1_ (size - 1)
        {
            begin_ = 0;
            end_ = size_;
            cursor_.store (-1);
        }

        /** waits for begin() to be valid and then
         *  returns it.  This is only safe for
         *  single producers, multi-producers should
         *  use claim(1) instead.
         */
        int64_t wait_next ()
        {
            wait_for (begin_);
            return begin_;
        }

        /**
         *   We need to wait until the available space in
         *   the ring buffer is pos - cursor which means that
         *   all readers must be at least to pos - size_ and
         *   that our new end is the min of the readers + size_
         */
        int64_t wait_for (int64_t pos)
        {
            try {
                // throws exception on error, returns 'short' on eof
                return end_ = barrier_.wait_for (pos - size_) + size_;
            }
            catch (...) {
                set_alert (std::current_exception ());
                throw;
            }
        }
        int64_t check_end ()
        {
            return end_ = barrier_.get_min () + size_;
        }

    private:
        const int64_t size_;
        const int64_t size_m1_;
    };
    typedef std::shared_ptr<WriteCursor> WriteCursorPtr;

    /**
     *  When there are multiple writers this cursor can
     *  be used to reserve space in the write buffer
     *  in an atomic manner.
     *
     *  @code
     *  auto start = cur->claim (slots);
     *  ... do your writes...
     *  cur->publish_after (start + slots, start - 1);
     *  @endcode
     */
    class SharedWriteCursor : public WriteCursor
    {
    public:
        /* @param size - the size of the ringbuffer, required to do proper wrap detection */
        SharedWriteCursor (int64_t size) :WriteCursor (size) 
        {
        }

        /**
         * @param name - name of the cursor for debug purposes
         * @param size - the size of the buffer.
         */
        SharedWriteCursor (const char* name, int64_t size) : WriteCursor (name, size) 
        {
        }

        /** 
         *  When there are multiple writers they cannot both
         *  assume the right to write to begin () to end (),
         *  instead they must first claim some slots in an
         *  atomic manner.
         *
         *  After pos().aquire() == claim(slots) - 1 the claimer
         *  is free to call publish up to start + slots - 1
         *
         *  @return the first slot the caller may write to.
         */
        int64_t claim (size_t num_slots)
        {
            auto pos = claim_cursor_.atomic_increment_and_get (num_slots);
            // make sure there is enough space to write
            wait_for (pos);
            return pos - num_slots;
        }

        void publish_after (int64_t pos, int64_t after_pos)
        {
            try {
                assert (pos > after_pos);
                barrier_.wait_for (after_pos);
                publish (pos);
            }
            catch (const Eof&) {
                cursor_.set_eof ();
                throw;
            }
            catch (...) {
                set_alert (std::current_exception ());
                throw;
            }
        }

    private:
        Sequence claim_cursor_;
    };
    typedef std::shared_ptr<SharedWriteCursor> SharedWriteCursorPtr;

    //////////////////////////////////////////////////////////////////////////
    //
    // Barrier function define
    //
    inline void Barrier::follows (std::shared_ptr<const EventCursor> event_cursor)
    {
        limit_seq_.push_back (std::move (event_cursor));
    }

    inline int64_t Barrier::get_min ()
    {
        int64_t min_pos = kMaxInt64Value;
        for (auto itr = limit_seq_.begin (); itr != limit_seq_.end (); ++itr) {
            auto itr_pos = (*itr)->pos ().aquire ();
            if (itr_pos < min_pos) {
                min_pos = itr_pos;
            }
        }

        return last_min_ = min_pos;
    }

    inline int64_t Barrier::wait_for (int64_t pos) const
    {
        if (last_min_ > pos) {
            return last_min_;
        }

        int64_t min_pos = kMaxInt64Value;
        for (auto itr = limit_seq_.begin (); itr != limit_seq_.end (); ++itr) {
            int64_t itr_pos = 0;
            itr_pos = (*itr)->pos ().aquire ();

            // spin for a bit 
            for (int i = 0; itr_pos < pos && i < 1000; ++i) {
                itr_pos = (*itr)->pos ().aquire ();
                if ((*itr)->pos ().alert ()) {
                    break;
                }
            }

            // yield for a while, queue slowing down
            for (int y = 0; itr_pos < pos && y < 1000; ++y) {
                usleep (0);

                itr_pos = (*itr)->pos ().aquire ();
                if ((*itr)->pos ().alert ()) {
                    break;
                }
            }

            // queue stalled, don't peg the CPU but don't wait too long either...
            while (itr_pos < pos) {
                usleep (10 * 1000);

                itr_pos = (*itr)->pos ().aquire ();
                if ((*itr)->pos ().alert ()) {
                    break;
                }
            }

            if ((*itr)->pos ().alert ()) {
                (*itr)->check_alert ();

                if (itr_pos > pos) {
                    // process everything up to itr_pos
                    return itr_pos - 1;
                }
                else {
                    throw Eof ();
                }
            }

            if (itr_pos < min_pos) {
                min_pos = itr_pos;
            }
        }

        assert (min_pos != kMaxInt64Value);
        return last_min_ = min_pos;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // EventCursor function define
    //
    inline void EventCursor::check_alert () const
    {
        if (alert_ != std::exception_ptr ()) {
            std::rethrow_exception (alert_);
        }
    }

} // namespace disruptor
} // namespace swift

#endif // __SWIFT_DISRUPTOR_DISRUPTOR_HPP__
