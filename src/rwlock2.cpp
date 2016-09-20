#include <iostream>
#include <map>
#include <string>
#include <boost/thread.hpp>

// This is not a good example.
// Just demo the usage of upgrade_lock.

class Cache {
public:
  ~Cache() {
    Clear();
  }

  void Clear() {
    // Exclusive ownership.
    boost::unique_lock<boost::shared_mutex> lock(shared_mutex_);

    object_map_.clear();
  }

  int GetOrCreate1(int key) {
    {
      // Acquire a shared ownership to read.
      boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);

      shared_mutex_.lock_shared();

      ObjectMap::iterator it = object_map_.find(key);
      if (it != object_map_.end()) {
        std::cout << "key already exists: " << key << std::endl;
        return it->second;
      }
    }

    {
      // Reacquire an exclusive ownership to write.
      boost::unique_lock<boost::shared_mutex> lock(shared_mutex_);
      object_map_[key] = 0;
    }

    return 0;
  }

  int GetOrCreate2(int key) {
    // Acquire shared ownership to read.
    boost::upgrade_lock<boost::shared_mutex> upgrade_lock(shared_mutex_);

    ObjectMap::iterator lb = object_map_.lower_bound(key);  // key <= lb->first
    if (lb != object_map_.end() && !(object_map_.key_comp()(key, lb->first))) {  // key >= lb->first
      std::cout << "key already exists: " << key << std::endl;
      return lb->second;
    }

    // Upgrade to exclusive ownership to write.
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(upgrade_lock);
    object_map_.insert(lb, std::make_pair(key, 0));

    return 0;
  }

private:
  typedef std::map<int, int> ObjectMap;
  ObjectMap object_map_;

  boost::shared_mutex shared_mutex_;
};

Cache g_cache;

void Worker() {
  for (int i = 0; i < 10; ++i) {
    g_cache.GetOrCreate1(i);
  }
}

int main() {
  boost::thread_group threads;

  for (size_t i = 0; i < 3; ++i) {
    threads.create_thread(Worker);
  }

  threads.join_all();

  return 0;
}

// Test map::lower_bound and map::key_comp.
void TestMap() {
  typedef std::map<int, std::string> MapType;
  MapType mymap;

  mymap[1] = "1";
  mymap[3] = "3";
  mymap[5] = "5";

  // key_comp() means "less than" by default.
  std::cout << mymap.key_comp()(1, 1) << std::endl;  // 0
  std::cout << mymap.key_comp()(1, 2) << std::endl;  // 1
  std::cout << mymap.key_comp()(2, 1) << std::endl;  // 0

  int key = 2;
  std::string value = "2";

  MapType::iterator lb = mymap.lower_bound(key);  // key <= lb->first
  if (lb != mymap.end() && !(mymap.key_comp()(key, lb->first))) {  // key >= lb->first
    // Key already exists.
    std::cout << "key already exits" << std::endl;
  } else {
    // Use lb as a hint to insert, so it can avoid another lookup.
    mymap.insert(lb, std::make_pair(key, value));
  }
}
