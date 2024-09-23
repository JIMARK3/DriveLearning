## DLL注入 + IATHOOK
### ExplorerSuite
#### Import Adder
` 增加PE文件导入表项 `
#### Add
` 选择DLL `
#### Import By Name
` 选择需要导入的DLL函数 `
#### Rebuid Import Table
` 重构PE文件Dll导入表 `
## MFCApplication_IAT.zip
` vs2013 mfc项目 输入32位程序pid 获得该进程加载的dll，和从该dll导入的func名，以及func的addr。
  原理 扫描导入表描述符和IAT,INT双桥结构。导入表描述符获取dll名，INT获取func名，IAT获取func的addr `
