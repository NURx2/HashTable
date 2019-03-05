#include <vector>
#include <utility>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
 private:
    std::vector<std::pair<KeyType, ValueType>> data;
    std::vector<bool> used;
    std::vector<bool> removed;
    Hash hasher;
    size_t curSize;
    size_t bigSize;  // includes removed elements, need for stopTheWorld

    size_t mustBe(const KeyType & key) const {
        return hasher(key) % data.size();
    }

    void stopTheWorld() {
        size_t newSize = 2 * data.size();
        std::vector<std::pair<KeyType, ValueType>> ndata(newSize);
        std::vector<bool> nused(newSize);
        std::vector<bool> nremoved(newSize);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[i]) {
                continue;
            }
            size_t pos = hasher(data[i].first) % newSize;
            while (nused[pos]) {
                pos = (pos + 1) % newSize;
            }
            nused[pos] = true;
            ndata[pos] = data[i];
        }
        swap(ndata, data);
        swap(used, nused);
        swap(removed, nremoved);
    }

 public:
    HashMap(Hash h = Hash()) : hasher(h) {
        curSize = 0;
        bigSize = 0;
        data.resize(1);
        used.resize(1);
        removed.resize(1);
    }

    template<typename Iter>
    HashMap(Iter begin, Iter end, Hash h = Hash()) : HashMap(h) {
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> & il,
        Hash h = Hash()) : HashMap(h) {
        for (auto & x : il) {
            insert(x);
        }
    }

    class iterator {
     friend class HashMap;  // когда нулы
     private:
        size_t i;
        HashMap * p;

        size_t next(size_t x) {
            ++x;
            for (; x < p->data.size(); ++x) {
                if (p->used[x]) {
                    break;
                }
            }
            return x;
        }

     public:
        iterator(HashMap * a = 0, size_t x = 0) : p(a) {
            if (p == 0) {
                return;
            }
            for (i = x; i < p->data.size(); ++i) {
                if (p->used[i]) {
                    break;
                }
            }
        }

        std::pair<const KeyType, ValueType> & operator * () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>&>(
                p->data[i]
            );
        }

        std::pair<const KeyType, ValueType> * operator -> () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>*>(
                &p->data[i]
            );
        }

        bool operator == (const iterator & a) const {
            return a.i == i && a.p == p;
        }

        bool operator != (const iterator & a) const {
            return !(*this == a);
        }

        iterator operator ++ (int) {
            int j = i;
            i = next(i);
            return iterator(p, j);
        }

        iterator & operator ++ () {
            i = next(i);
            return *this;
        }
    };

    class const_iterator {
     friend class HashMap;
     private:
        size_t i;
        const HashMap * p;

        size_t next(size_t x) {
            ++x;
            for (; x < p->data.size(); ++x) {
                if (p->used[x]) {
                    break;
                }
            }
            return x;
        }

     public:
        const_iterator(const HashMap * a = 0, size_t x = 0) : p(a) {
            if (p == 0) {
                return;
            }
            for (i = x; i < p->data.size(); ++i) {
                if (p->used[i]) {
                    break;
                }
            }
        }
        
        const std::pair<const KeyType, ValueType> & operator * () const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>&>(
                p->data[i]
            );
        }
        
        const std::pair<const KeyType, ValueType> * operator -> () const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(
                &p->data[i]
            );
        }
        
        bool operator == (const const_iterator & a) const {
            return a.i == i && a.p == p;
        }

        bool operator != (const const_iterator & a) const {
            return !(*this == a);
        }

        const_iterator operator ++ (int) {
            int j = i;
            i = next(i);
            return const_iterator(p, j);
        }

        const_iterator & operator ++ () {
            i = next(i);
            return *this;
        }
    };

    iterator begin() {
        return iterator(this, 0);
    }

    iterator end() {
        return iterator(this, data.size());
    }

    const_iterator begin() const {
        return const_iterator(this, 0);
    } 

    const_iterator end() const {
        return const_iterator(this, data.size());
    }

    void insert(const std::pair<KeyType, ValueType> & p) {
        ++curSize;
        ++bigSize;
        if (2 * bigSize >= data.size()) {
            stopTheWorld();
        }
        size_t pos = mustBe(p.first);
        bool already = false;
        for (size_t i = 0; i < data.size(); ++i) {
            if (used[pos] && data[pos].first == p.first) {
                already = true;
                --curSize;
                break;
            }
            if (!used[pos]) {
                break;
            }
            pos = (pos + 1) % data.size();
        }
        if (!already) {
            used[pos] = true;
            removed[pos] = false;
            data[pos] = p;
        }
    }

    void erase(const KeyType & key) {
        size_t pos = mustBe(key);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[pos] && !removed[pos]) {
                break;
            }
            if (used[pos] && data[pos].first == key) {
                removed[pos] = true;
                used[pos] = false;
                --curSize;
                break;
            }
            pos = (pos + 1) % data.size();
        }
    }

    iterator find(const KeyType & key) {
        size_t pos = mustBe(key);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[pos] && !removed[pos]) {
                break;
            }
            if (used[pos] && data[pos].first == key) {
                return iterator(this, pos);
            }
            pos = (pos + 1) % data.size();
        }
        return end();
    }

    const_iterator find(const KeyType & key) const {
        size_t pos = mustBe(key);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[pos] && !removed[pos]) {
                break;
            }
            if (used[pos] && data[pos].first == key) {
                return const_iterator(this, pos);
            }
            pos = (pos + 1) % data.size();
        }
        return end();
    }

    ValueType & operator [] (const KeyType & key) {
        size_t pos = mustBe(key);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[pos] && !removed[pos]) {
                break;
            }
            if (used[pos] && data[pos].first == key) {
                return data[pos].second;
            }
            pos = (pos + 1) % data.size();
        }
        insert({key, ValueType()});
        return (*this)[key];
    }

    const ValueType & at(const KeyType & key) const {
        size_t pos = mustBe(key);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[pos] && !removed[pos]) {
                break;
            }
            if (used[pos] && data[pos].first == key) {
                return data[pos].second;
            }
            pos = (pos + 1) % data.size();
        }
        throw std::out_of_range("");
    }

    void clear() {
        curSize = 0;
        bigSize = 0;
        data.clear();
        used.clear();
        removed.clear();
        data.assign(1, std::make_pair(KeyType(), ValueType()));
        used.assign(1, 0);
        removed.assign(1, 0);
    }

    Hash hash_function() const {
        return hasher;
    }

    size_t size() const {
        return curSize;
    }

    size_t empty() const {
        return size() == 0;
    }
};