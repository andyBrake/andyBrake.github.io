# C++ 插件安装和配置指南

## 🔧 问题：鼠标右键没有"跳转到定义"等功能

这是因为 C++ 插件还未安装。请按以下步骤操作：

---

## 📦 第一步：安装 C++ 插件

### 方法 1：通过推荐安装（最简单）✅
1. VSCode 右下角会弹出提示：
   ```
   This workspace has extension recommendations.
   ```
2. 点击 **"Install All"** 或 **"Show Recommendations"**
3. 安装 **C/C++** 插件（作者：Microsoft）

### 方法 2：手动安装
1. 按 `Cmd+Shift+X` 打开扩展面板
2. 搜索 **"C/C++"**
3. 安装第一个（作者：Microsoft，图标是蓝色的 C++）
4. 点击 **"Install"**

---

## ⏱️ 第二步：等待索引完成

安装后，插件需要索引你的代码：

1. 右下角会显示 **"Indexing..."** 或 **"Parsing..."**
2. 等待进度条完成（通常 10-30 秒）
3. 完成后右下角会显示 **"Ready"** 或不显示任何内容

**注意：** 在索引完成之前，右键菜单功能不可用！

---

## ✅ 第三步：验证功能

索引完成后，鼠标右键应该会出现：

- ✅ **Go to Definition** (跳转到定义)
- ✅ **Go to Declaration** (跳转到声明)
- ✅ **Peek Definition** (预览定义) - 也可以用 `Cmd+Z`
- ✅ **Find All References** (查找所有引用)
- ✅ **Rename Symbol** (重命名符号)

---

## 🐛 如果还是没有？

### 检查 1：确认插件已启用
1. 按 `Cmd+Shift+X` 打开扩展面板
2. 搜索 "C/C++"
3. 确保显示 **"Enabled"** 而不是 "Disabled"

### 检查 2：重新加载窗口
1. 按 `Cmd+Shift+P` 打开命令面板
2. 输入 "Reload Window"
3. 回车重新加载

### 检查 3：查看 IntelliSense 状态
在 `.h` 或 `.cpp` 文件中：
1. 点击右下角的 **"C/C++"** 状态栏
2. 查看是否显示 "Ready" 或错误信息
3. 如果显示错误，点击查看详情

### 检查 4：手动触发索引
1. 按 `Cmd+Shift+P`
2. 输入 "C/C++: Rescan Workspace"
3. 回车重新扫描

---

## 🎯 预期效果

完成后，在代码中：

```cpp
dealer.dealHoleCards(privateCard);
       ↑
    右键点击这里
```

会出现菜单：
```
✓ Go to Definition
✓ Peek Definition
✓ Go to Type Definition
✓ Find All References
✓ Rename Symbol
```

---

## 💡 提示

- 第一次打开项目时，索引可能需要 1-2 分钟
- 大型项目索引时间更长，请耐心等待
- 索引期间可以继续编辑代码，但智能提示功能会受限

---

## 📞 需要帮助？

如果按照以上步骤仍然无法解决，请：
1. 查看 VSCode 输出面板（`Cmd+Shift+U`）
2. 选择 "C/C++" 频道
3. 查看是否有错误信息
