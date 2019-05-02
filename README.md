# HashMap
Own C++17 hashtable header
## Usage examples
#### Including of the header file
```cpp
#include <HashMap.h>
```
### > Variable declaration
```cpp
HashMap<int, int> myHashmap{{1, 2}, {0, 0}};
```
#### Correctly works with constness
```cpp
const HashMap<int, int> myConstHashmap{{1, 5}, {3, 4}, {2, 1}};
```
### > Usage of iterators
```cpp
HashMap<int, int>::iterator it = myHashmap.end();
HashMap<int, int>::const_iterator it2 = myConstHashmap.begin();
for (auto cur : myHashmap) {
    std::cout << cur.first << ' ' << cur.second << "\n";
}
```
#### .find() implementation
```cpp
const HashMap<int, int> myConstHashmap{{1, 5}, {3, 4}, {2, 1}};
HashMap<int, int>::const_iterator it;
it = myConstHashmap.find(3);  // it->second == 4
it = myConstHashmap.find(7);  // it->second == myConstHashMap.end()
```