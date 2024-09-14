#### 环境的主机系统
`WIN10`
#### DBGPRINT 没有输出
`在注册表HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/Session Manager/中
新建key,名字为Debug Print Filter ，然后在此key下新建一个DWORD value ，名字为DEFAULT,然后设置值为0x00000008,
重启电脑后会生效。`
