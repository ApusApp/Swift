#ifndef __SWIFT_BASE_LINKED_HASH_MAP_H__
#define __SWIFT_BASE_LINKED_HASH_MAP_H__

#include <hash_map>
#include <limits> // numeric_limits
#include <sys/mman.h>
#include <assert.h>

#include "swift/base/noncopyable.hpp"

namespace swift {

/**
 * Doubly-linked hash map
 * @param KEY the key type
 * @param VALUE the value type
 * @param HASH the hash functor
 * @param EQUALTO the equality checking functor
 */
template <class KEY, 
          class VALUE,
          class HASH = std::hash<KEY>, 
          class EQUALTO = std::equal_to<KEY> >
class LinkedHashMap : swift::noncopyable
{
private:
    struct Record
    {
        KEY key_;		// key
        VALUE value_;	// value
        Record* child_;  // child record
        Record* prev_;	// previous record
        Record* next_;	// next record

        explicit Record (const KEY& k, const VALUE& v)
            : key_ (k)
            , value_ (v)
            , child_ (nullptr)
            , prev_ (nullptr)
            , next_ (nullptr)
        {
        }
    };

    //
    // The default bucket number of hash table
    //
    static const size_t MAP_DEFAULT_BUCKET_NUM = 31;

    //
    // The mininum number of buckets to use mmap 
    //
    static const size_t MIN_MAPZMAP_BUCKET_NUM = 32768;

public:
    /**
    * Moving Modes
    */
    enum MoveMode
    {
        MM_CURRENT,
        MM_FIRST,
        MM_LAST
    };

    //
    // Iterator of records
    //
    class Iterator
    {
    public:
        /**
        * Copy constructor
        *
        * @param [in] iter the source object
        */
        Iterator (const Iterator& iter)
            : map_ (iter.map_)
            , rec_ (iter.rec_)
        {
        }

        /**
        * Get the key
        *
        * @return a reference of the key
        */
        const KEY& Key ()
        {
            return rec_->key_;
        }

        /**
        * Get the value
        *
        * @return a reference of the value
        */
        VALUE& Value ()
        {
            return rec_->value_;
        }

        /**
        * Assignment operator from the self type
        *
        * @param [in] rhs the right operand
        *
        * @return the reference to itself
        */
        Iterator& operator= (const Iterator& rhs)
        {
            if (&rhs != this) {
                map_ = rhs.map_;
                rec_ = rhs.rec_;
            }

            return *this;
        }

            /**
            * Equality operator with the self type
            *
            * @param [in] rhs the right operand
            *
            * @return true if the both are equal, or false if not
            */
        bool operator== (const Iterator& rhs) const 
        {
            return ((map_ == rhs.map_) && (rec_ == rhs.rec_));
        }

        /**
        * Noe-equality operator with the self type
        *
        * @param [in] rhs the right operand
        *
        * @return false if the both are equal, or true if not
        */
        bool operator!= (const Iterator& rhs) const
        {
            return ((map_ != rhs.map_) || (rec_ != rhs.rec_));
        }

        /**
        * Preposting increment operator
        *
        * @return the iterator itself
        */
        Iterator& operator++ ()
        {
            rec_ = rec_->next_;

            return *this;
        }

        /**
        * Postposting decrement operator
        *
        * @return an iterator of the old position
        */
        Iterator operator++ (int)
        {
            Iterator old (*this);
            rec_ = rec_->next_;

            return old;
        }

            /**
            * Preposting decrement operator
            * 
            * @return the iterator itself
            */
        Iterator& operator-- ()
        {
            if (rec_) {
                rec_ = rec_->prev_;
            }
            else {
                rec_ = map_->last_;
            }

            return *this;
        }

        /**
        * Postposing decrement operator
        *
        * @return an iterator of the old position
        */
        Iterator operator-- (int)
        {
            Iterator old (*this);
            if (rec_) {
                rec_ = rec_->prev_;
            }
            else {
                rec_ = map_->last_;
            }

            return old;
        }

    private:
        /**
        * Constructor
        * 
        * @param [in] map the container
        * @param [in] rec the pointer to the current record
        */
        explicit Iterator (LinkedHashMap* map, Record* rec)
            : map_ (map)
            , rec_ (rec)
        {

        }

        friend class LinkedHashMap;
        LinkedHashMap* map_;	// the container
        Record* rec_;			// the current record
    };

public:
    /**
    * Default constructor
    */
    explicit LinkedHashMap ()
        : buckets_ (nullptr)
        , bnum_ (MAP_DEFAULT_BUCKET_NUM)
        , first_ (nullptr)
        , last_ (nullptr)
        , count_ (0)
    {
        if (bnum_ < 1) {
            bnum_ = MAP_DEFAULT_BUCKET_NUM;
        }

        Initialize ();
    }

    /**
    * Constructor
    *
    * @param [in] bnum the number of buckets of the hash table
    */
    explicit LinkedHashMap (size_t bnum)
        : buckets_ (nullptr)
        , bnum_ (bnum)
        , first_ (nullptr)
        , last_ (nullptr)
        , count_ (0)
    {
        if (bnum_ < 1) {
            bnum_ = MAP_DEFAULT_BUCKET_NUM;
        }

        Initialize ();
    }

    /**
    * Destructor
    */
    ~LinkedHashMap ()
    {
        Destroy ();
    }

    /**
    * Store a record
    *
    * @param [in] key the key
    * @param [in] value the value
    * @param [in] mode the moving mode
    *
    * @return the pointer to the value of the stored record
    */
    VALUE* Set (const KEY& key, const VALUE& value, MoveMode mode)
    {
        size_t bidx = hash_ (key) % bnum_;
        Record* rec = buckets_[bidx];
        Record** entp = buckets_ + bidx;

        while (rec) {
            if (_equalto (rec->key_, key)) {
                rec->value_ = value;
                switch (mode) {
                default:
                {
                    break;
                }
                case MM_FIRST:
                {
                    if (first_ != rec) {
                        if (last_ == rec) last_ = rec->prev_;
                        if (rec->prev_)    rec->prev_->next_ = rec->next_;
                        if (rec->next_)    rec->next_->prev_ = rec->prev_;
                        rec->prev_ = 0;
                        rec->next_ = first_;
                        first_->prev_ = rec;
                        first_ = rec;
                    }

                    break;
                }
                case MM_LAST:
                {
                    if (last_ != rec) {
                        if (first_ == rec) {
                            first_ = rec->next_;
                        }

                        if (rec->prev_) {
                            rec->prev_->next_ = rec->next_;
                        }

                        if (rec->next_) {
                            rec->next_->prev_ = rec->prev_;
                        }

                        rec->prev_ = nullptr;
                        rec->next_ = first_;
                        first_->prev_ = rec;
                        first_ = rec;
                    }

                    break;
                } // end MM_LAST
                } // end switch  (mode)

                return &rec->value_;
            } // end if _equalto (rec->key, key)
            else {
                entp = &rec->child_;
                rec = rec->child_;
            }
        } // end while (rec)

        rec = new Record (key, value);
        switch (mode) {
        default:
        {
            rec->prev_ = last_;
            if (!first_) first_ = rec;
            if (last_)   last_->next_ = rec;
            last_ = rec;

            break;
        }
        case MM_FIRST:
        {
            rec->next_ = first_;
            if (!last_) last_ = rec;
            if (first_) first_->prev_ = rec;
            first_ = rec;

            break;
        }
        } // end switch mode

        *entp = rec;
        ++count_;

        return &rec->value_;
    } // end Set function

    /**
    * Remove a record
    *
    * @param [in] key the key
    *
    * @return true on success, of false on failure
    */
    bool Remove (const KEY& key)
    {
        size_t bidx = hash_ (key) % bnum_;
        Record* rec = buckets_[bidx];
        Record** entp = buckets_ + bidx;
        while (rec) {
            if (_equalto (rec->key_, key)) {
                if (rec->prev_)     rec->prev_->next_ = rec->next_;
                if (rec->next_)     rec->next_->prev_ = rec->prev_;
                if (rec == first_) first_ = rec->next_;
                if (rec == last_)  last_ = rec->prev_;
                *entp = rec->child_;
                --count_;
                delete rec;
                rec = nullptr;

                return true;
            }
            else {
                entp = &rec->child_;
                rec = rec->child_;
            }
        } // end while rec

        return false;
    }

    /**
    * Migrate a record to another map
    *
    * @param [in] key the key
    * @param [in] dist the destination map
    * @param [in] mode the moving mode
    * 
    * @return the pointer to the value of the migrated record, or zero on failure
    */
    VALUE* Migrate (const KEY& key, 
                    LinkedHashMap* dist, 
                    MoveMode mode)
    {
        size_t hash = hash_ (key);
        size_t bidx = hash % bnum_;
        Record* rec = buckets_[bidx];
        Record** entp = buckets_ + bidx;
        while (rec) {
            if (_equalto (rec->key_, key)) {
                if (rec->prev_)		rec->prev_->next_ = rec->next_;
                if (rec->next_)		rec->next_->prev_ = rec->prev_;
                if (rec == first_)	first_ = rec->next_;
                if (rec == last_)	last_ = rec->prev_;
                *entp = rec->child_;
                --count_;
                rec->child_ = nullptr;
                rec->prev_ = nullptr;
                rec->next_ = nullptr;

                bidx = hash % dist->bnum_;
                Record* drec = dist->buckets_[bidx];
                entp = dist->buckets_ + bidx;

                while (drec) {
                    if (dist->_equalto (drec->key_, key)) {
                        if (drec->child_) rec->child_ = drec->child_;

                        if (drec->prev_) {
                            rec->prev_ = drec->prev_;
                            rec->prev_->next_ = rec;
                        }

                        if (drec->next_) {
                            rec->next_ = drec->next_;
                            rec->next_->prev_ = rec;
                        }

                        if (dist->first_ == drec) dist->first_ = rec;
                        if (dist->last_ == drec)  dist->last_ = rec;
                        *entp = rec;
                        delete drec;
                        drec = nullptr;

                        switch (mode) {
                        default:
                        {
                            break;
                        }
                        case MM_FIRST:
                        {
                            if (dist->first_ != rec) {
                                if (dist->last_ == rec) dist->last_ = rec->prev_;
                                if (rec->prev_)			rec->prev_->next_ = rec->next_;
                                if (rec->next_)			rec->next_->prev_ = rec->prev_;
                                rec->prev_ = nullptr;
                                rec->next_ = dist->first_;
                                dist->first_->prev_ = rec;
                                dist->first_ = rec;
                            }

                            break;
                        }
                        case MM_LAST:
                        {
                            if (dist->last_ != rec) {
                                if (dist->first_ == rec) dist->first_ = rec->next_;
                                if (rec->prev_)			 rec->prev_->next_ = rec->next_;
                                if (rec->next_)			 rec->next_->prev_ = rec->prev_;
                                rec->prev_ = dist->last_;
                                rec->next_ = nullptr;
                                dist->last_->next_ = rec;
                                dist->last_ = rec;
                            }

                            break;
                        }
                        } // end switch (mode)

                        return &rec->value;
                    } // end if (dist->_equalto (drec->key, key))

                    entp = &drec->child_;
                    drec = drec->child_;
                } // end while (drec)

                switch (mode) {
                default:
                {
                    rec->prev_ = dist->last_;
                    if (!dist->first_) dist->first_ = rec;
                    if (dist->last_)   dist->last_->next_ = rec;
                    dist->last_ = rec;

                    break;
                }
                case MM_FIRST:
                {
                    rec->next_ = dist->first_;
                    if (!dist->last_) dist->last_ = rec;
                    if (dist->first_) dist->first_->prev_ = rec;
                    dist->first_ = rec;

                    break;
                }
                } // end switch (mode)

                *entp = rec;
                ++dist->count_;

                return &rec->value_;
            } // end if _equalte (rec->key, key)
            else {
                entp = &rec->child_;
                rec = rec->child_;
            }
        } // end while (rec)

        return nullptr;
    }

    /**
    * Retrieve a record
    *
    * @param [in] key the key
    * @param [in] mode the moving mode
    *
    * @return the pointer to the value of the corresponding record, or zero on failure
    */
    VALUE* Get (const KEY& key, MoveMode mode)
    {
        size_t bidx = hash_ (key) % bnum_;
        Record* rec = buckets_[bidx];

        while (rec) {
            if (_equalto (rec->key_, key)) {
                switch (mode) {
                default:
                {
                    break;
                }
                case MM_FIRST:
                {
                    if (first_ != rec) {
                        if (last_ == rec) last_ = rec->prev_;
                        if (rec->prev_)	  rec->prev_->next_ = rec->next_;
                        if (rec->next_)	  rec->next_->prev_ = rec->prev_;
                        rec->prev_ = nullptr;
                        rec->next_ = first_;
                        first_->prev_ = rec;
                        first_ = rec;
                    }

                    break;
                }
                case MM_LAST:
                {
                    if (last_ != rec) {
                        if (first_ == rec) first_ = rec->next_;
                        if (rec->prev_)	   rec->prev_->next_ = rec->next_;
                        if (rec->next_)	   rec->next_->prev_ = rec->prev_;
                        rec->prev_ = last_;
                        rec->next_ = nullptr;
                        last_->next_ = rec;
                        last_ = rec;
                    }

                    break;
                }
                } // end switch (mode)

                return &rec->value_;
            } // end if (_equalto (rec->key, key))
            else {
                rec = rec->child_;
            }
        } // end while (rec)

        return nullptr;
    } // end Get

    /**
    * Remove all records
    */
    void Clear ()
    {
        if (count_ < 1) return;

        Record* rec = last_;
        while (rec) {
            Record* prev = rec->prev_;
            delete rec;
            rec = prev;
        }

        for (size_t i = 0; i < bnum_; ++i) {
            buckets_[i] = nullptr;
        }

        first_ = nullptr;
        last_ = nullptr;
        count_ = nullptr;
    } // end Clear Function

    /**
    * Get the number of records
    */
    size_t Count () const
    {
        return count_;
    }

    /**
    * Get an iterator at the first record
    */
    Iterator Begin ()
    {
        return Iterator (this, first_);
    }

    /**
    * Get an iterator of the end sentry.
    */
    Iterator End ()
    {
        return Iterator(this, nullptr);
    }

    /**
    * Get an iterator at a record
    *
    * @param [in] key the key
    *
    * @return the pointer to the value of the corresponding record, of zero on failure
    */
    Iterator Find (const KEY& key)
    {
        Record* rec = buckets_[hash_ (key) % bnum_];
        while (rec) {
            if (_equalto (rec->key_, key)) {
                return Iterator (this, rec);
            }
            else {
                rec = rec->child_;
            }
        }

        return Iterator (this, nullptr);

    }

    /**
    * Get the reference of the key of the first record
    *
    * @return the reference of the key of the first record
    */
    const KEY& FirstKey () const
    {
        return first_->key_;
    }

    /**
    * Get the reference of the value of the first record
    *
    * @return the reference of the value of the first record
    */
    VALUE& FirstValue () const
    {
        return first_->value_;
    }

    /**
    * Get the reference of the key of the last record
    *
    * @return the reference of the key of the last record
    */
    const KEY& LastKey () const
    {
        return last_->key_;
    }

        /**
        * Get the reference of the value of the last record
        *
        * @return the reference of the value of the last record
        */
    VALUE& LastValue () const
    {
        return last_->value_;
    }

private:

    inline void *MapAlloc (size_t size)
    {
        assert (size > 0 && size <= std::numeric_limits<size_t>::max () / 2);
        void* ptr = ::mmap (0, sizeof(size) + size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (MAP_FAILED == ptr) {
            throw std::bad_alloc ();
        }

        *(size_t*)ptr = size;
        return (char*)ptr + sizeof(size);
    }

    inline void MapFree (void* ptr) 
    {
        if (ptr) {
            size_t size = *((size_t*)ptr - 1);
            ::munmap ((char*)ptr - sizeof(size), sizeof(size) + size);
        }
    }

    /**
    * Initialize fields
    */
    void Initialize ()
    {
        if (bnum_ >= MIN_MAPZMAP_BUCKET_NUM) {
                buckets_ = (Record**)MapAlloc (sizeof(*buckets_) * bnum_);
        }
        else {
            buckets_ = new Record*[bnum_];
            for (size_t i = 0; i < bnum_; ++i) {
                buckets_[i] = nullptr;
            }
        }
    }

    /**
    * Clean up fields
    */
    void Destroy ()
    {
        Record* rec = last_;
        while (rec) {
            Record* prev = rec->prev_;
            delete rec;
            rec = prev;
        }

        if (bnum_ >= MIN_MAPZMAP_BUCKET_NUM) {
            MapFree (buckets_);
        }
        else {
            if (buckets_) {
                delete [] buckets_;
                buckets_ = nullptr;
            }
        }
    }

private:
    HASH hash_;			// the functor of the hash function
    EQUALTO _equalto;	// the functor of the equalto function
    Record** buckets_;  // the backet array
    Record* first_;		// the first record
    Record* last_;		// the lash record
    size_t bnum_;		// the number of backets
    size_t count_;		// the number of records

};

} // namespace swift
#endif //__SWIFT_BASE_LINKED_HASH_MAP_H__
