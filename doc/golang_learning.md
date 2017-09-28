# 纪要
 * `[n]T`或者`[...]T{X, X}`这种在声明时指定大小的是array类型，它是大小恒定，不支持resize操作的. `[]type` 这种声明不指定大小的是slice类型，它支持动态变化大小，切片，append等各种操作 
 * flow control 语句中的()都是可以省略的，比如 `if a > 0`，但是{}不可以省略，必须有，即使条件内执行语句只有一句，这和c语言的规定是不一样的。
 * slice 是array的部分区间的引用，修改slice元素的值就会改动array对应的值。slice的数据结构实际是一个描述结构，内有一个指针指向对应array，另外两个字段分别是len和cap。
 ```
 The length of a slice is the number of elements it contains.

 The capacity of a slice is the number of elements in the underlying array, counting from the first element in the slice.

 The length and capacity of a slice s can be obtained using the expressions len(s) and cap(s).
```

 * [闭包函数](https://tour.golang.org/moretypes/25)
 * method才支持入参的指针和值自动转换，普通func不支持。
