# git 操作总结

1. 设置git的对比工具和mergetool为beyond compare

#difftool 配置  
```
git config --global diff.tool bc4  
git config --global difftool.bc4.cmd "\"c:/program files (x86)/beyond compare 4/bcomp.exe\" \"$LOCAL\" \"$REMOTE\""  
```

#mergeftool 配置 
```
git config --global merge.tool bc4  
git config --global mergetool.bc4.cmd  "\"c:/program files (x86)/beyond compare 4/bcomp.exe\" \"$LOCAL\" \"$REMOTE\" \"$BASE\" \"$MERGED\""  
git config --global mergetool.bc4.trustExitCode true  
```
  
#让git mergetool不再生成备份文件（*.orig）  
`git config --global mergetool.keepBackup false  `

2. git的用户名和email
```
$ git config　user.name nickname#将用户名设为nickname   huangfa
$ git config　user.email nickname@gmail.com #将用户邮箱设为cavalier_man@163.com
```

3. 使用命令 `ssh-keygen` 产生ssh的公钥和私钥，输出会提示公钥文件位置，打开公钥文件 `id_rsa.pub` 将里面的全部内容复制出来，拷贝到github的配置页面的新增ssh key里面即可。
