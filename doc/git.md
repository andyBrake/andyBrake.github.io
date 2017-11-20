# git 操作总结

1. 设置git的对比工具和mergetool为beyond compare

```
#difftool 配置  
git config --global diff.tool bc4  
git config --global difftool.bc4.cmd "\"c:/program files (x86)/beyond compare 4/bcomp.exe\" \"$LOCAL\" \"$REMOTE\""  
    
#mergeftool 配置  
git config --global merge.tool bc4  
git config --global mergetool.bc4.cmd  "\"c:/program files (x86)/beyond compare 4/bcomp.exe\" \"$LOCAL\" \"$REMOTE\" \"$BASE\" \"$MERGED\""  
git config --global mergetool.bc4.trustExitCode true  
  
#让git mergetool不再生成备份文件（*.orig）  
git config --global mergetool.keepBackup false  
```

git的用户名和email
$ git config　user.name nickname#将用户名设为nickname   huangfa
$ git config　user.email nickname@gmail.com #将用户邮箱设为cavalier_man@163.com
