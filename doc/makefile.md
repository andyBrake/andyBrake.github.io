如何在makefile中调用python脚本，并使该脚本的print输出作为makefile中变量的值：
```
TOKEN_OBJS_C             = $(shell python ./get_files_name.py)
```
其中`get_files_name.py`这个脚本会print一个string到屏幕，这个输出就会作为变量`TOKEN_OBJS_C`的值
