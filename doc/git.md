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

3. [gen ssh key](https://git-scm.com/book/zh/v2/%E6%9C%8D%E5%8A%A1%E5%99%A8%E4%B8%8A%E7%9A%84-Git-%E7%94%9F%E6%88%90-SSH-%E5%85%AC%E9%92%A5)


4. 使用git进行局域网共享：
	http://blog.csdn.net/leichelle/article/details/27236443
	http://blog.csdn.net/elloop/article/details/54898512

