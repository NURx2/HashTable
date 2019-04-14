#include <vector>
#include <utility>
#include <stdexcept>

#include <iostream>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
 private:
    std::vector<std::pair<KeyType, ValueType>> data;
    std::vector<bool> used;
    std::vector<bool> removed;
    Hash hasher;
    size_t cntUsedNotRemoved;
    size_t cntAdded;

    size_t keyToValidIndex(const KeyType & key) const {
        return hasher(key) % data.size();
    }

    void reload() {
        size_t newSize = 2 * data.size();
        std::vector<std::pair<KeyType, ValueType>> newData(newSize);
        std::vector<bool> newUsed(newSize);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[i]) {
                continue;
            }
            size_t pos = hasher(data[i].first) % newSize;
            while (newUsed[pos]) {
                pos = (pos + 1) % newSize;
            }
            newUsed[pos] = true;
            newData[pos] = data[i];
        }
        swap(data, newData);
        swap(used, newUsed);
        removed.assign(newSize, 0);
    }

 public:
    HashMap(Hash h = Hash()) : hasher(h) {
        cntUsedNotRemoved = 0;
        cntAdded = 0;
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

    HashMap(
        std::initializer_list<std::pair<KeyType, ValueType>> il,
        Hash h = Hash()
    ) : HashMap(h) {
        for (auto & x : il) {
            insert(x);
        }
    }

    template <typename HMtype>
    class innerIterator {
     friend class HashMap;
     protected:
        size_t elementIndex;
        HMtype * owner;

     private:
        size_t next(size_t curInd) {
            ++curInd;
            for (; curInd < owner->data.size(); ++curInd) {
                if (owner->used[curInd]) {
                    break;
                }
            }
            return curInd;
        }

     public:
        explicit innerIterator(HMtype * owner = 0, size_t beg = 0) : owner(owner) {
            if (this->owner == 0) {
                return;
            }
            for (elementIndex = beg; elementIndex < this->owner->data.size(); ++elementIndex) {
                if (this->owner->used[elementIndex]) {
                    break;
                }
            }
        }

        bool operator == (const innerIterator & a) const {
            return a.elementIndex == elementIndex && a.owner == owner;
        }

        bool operator != (const innerIterator & a) const {
            return !(*this == a);
        }

        innerIterator operator ++ (int) {
            int j = elementIndex;
            elementIndex = next(elementIndex);
            return innerIterator(owner, j);
        }

        innerIterator & operator ++ () {
            elementIndex = next(elementIndex);
            return *this;
        }
    };

    class iterator : public innerIterator<HashMap> {
    using innerIterator<HashMap>::owner;
    using innerIterator<HashMap>::elementIndex;
    using innerIterator<HashMap>::innerIterator;

     public:
        std::pair<const KeyType, ValueType> & operator * () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>&>(
                owner->data[elementIndex]
            );
        }

        std::pair<const KeyType, ValueType> * operator -> () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>*>(
                &owner->data[elementIndex]
            );
        }
    };

    class const_iterator : public innerIterator<const HashMap> {
    using innerIterator<const HashMap>::owner;
    using innerIterator<const HashMap>::elementIndex;
    using innerIterator<const HashMap>::innerIterator;

     public:
        const std::pair<const KeyType, ValueType> & operator * () const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>&>(
                owner->data[elementIndex]
            );
        }
        
        const std::pair<const KeyType, ValueType> * operator -> () const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(
                &owner->data[elementIndex]
            );
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
        ++cntUsedNotRemoved;
        ++cntAdded;
        if (2 * cntAdded >= data.size()) {
            reload();
        }
        size_t pos = keyToValidIndex(p.first);
        bool already = false;
        for (size_t i = 0; i < data.size(); ++i) {
            if (used[pos] && data[pos].first == p.first) {
                already = true;
                --cntUsedNotRemoved;
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
        size_t pos = keyToValidIndex(key);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[pos] && !removed[pos]) {
                break;
            }
            if (used[pos] && data[pos].first == key) {
                removed[pos] = true;
                used[pos] = false;
                --cntUsedNotRemoved;
                break;
            }
            pos = (pos + 1) % data.size();
        }
    }

    size_t innerFind(const KeyType & key) const {
        size_t pos = keyToValidIndex(key);
        for (size_t i = 0; i < data.size(); ++i) {
            if (!used[pos] && !removed[pos]) {
                break;
            }
            if (used[pos] && data[pos].first == key) {
                return pos;
            }
            pos = (pos + 1) % data.size();
        }
        return data.size();
    }

    iterator find(const KeyType & key) {
        return iterator(this, innerFind(key));
    }

    const_iterator find(const KeyType & key) const {
        return const_iterator(this, innerFind(key));
    }

    ValueType & operator [] (const KeyType & key) {
        size_t foundPos = innerFind(key);
        if (foundPos != data.size()) {
            return data[foundPos].second;
        }
        insert({key, ValueType()});
        return (*this)[key];
    }

    const ValueType & at(const KeyType & key) const {
        size_t foundPos = innerFind(key);
        if (foundPos != data.size()) {
            return data[foundPos].second;
        }
        throw std::out_of_range("");
    }

    void clear() {
        cntUsedNotRemoved = 0;
        cntAdded = 0;
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
        return cntUsedNotRemoved;
    }

    size_t empty() const {
        return size() == 0;
    }
};