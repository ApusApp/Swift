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

#ifndef __SWIFT_BASE_LRU_CACHE_HPP__
#define __SWIFT_BASE_LRU_CACHE_HPP__

#include <list>
#include <unordered_map>
#include <mutex>

namespace swift {

template<typename KeyType, typename ValueType>
class LruCache {
 private:
  typedef typename std::pair<KeyType, ValueType> KeyValuePairType;
  typedef typename std::list<KeyValuePairType>::iterator ListIteratorType;

 public:
  LruCache(size_t capacity);
  void Set(const KeyType& key, const ValueType& value);
  bool Get(const KeyType& key, ValueType& value);

private:
  std::mutex mutex_;
  size_t capacity_;
  std::list<KeyValuePairType> cache_items_list_;
  std::unordered_map<KeyType, ListIteratorType> cache_items_map_;
};

template<typename KeyType, typename ValueType>
swift::LruCache<KeyType, ValueType>::LruCache(size_t capacity) 
  : capacity_(capacity) {
}

template<typename KeyType, typename ValueType>
void swift::LruCache<KeyType, ValueType>::Set(const KeyType& key, const ValueType& value) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = cache_items_map_.find(key);
  if (it != cache_items_map_.end()) {
    cache_items_list_.erase(it->second);
    cache_items_map_.erase(it);
  }

  cache_items_list_.push_front(KeyValuePairType(key, value));
  cache_items_map_[key] = cache_items_list_.begin();

  if (cache_items_map_.size() > capacity_) {
    cache_items_map_.erase(cache_items_list_.back().first);
    cache_items_list_.pop_back();
  }
}

template<typename KeyType, typename ValueType>
bool swift::LruCache<KeyType, ValueType>::Get(const KeyType& key, ValueType& value) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = cache_items_map_.find(key);
  if (it == cache_items_map_.end()) {
    return false;
  } else {
    cache_items_list_.splice(cache_items_list_.begin(), cache_items_list_, it->second);
    value = it->second->second;
    return true;
  }
}

} // namespace swift

#endif // __SWIFT_BASE_LRU_CACHE_HPP__
