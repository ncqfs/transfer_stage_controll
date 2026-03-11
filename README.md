# transfer_stage_controll
# 显微镜控制系统

本项目是一个用于控制显微镜位移台、相机和自动对焦的 Windows 桌面应用程序。代码使用 C++ 编写，基于 Visual Studio 开发，依赖 OpenCV 和 IDS 相机 SDK。

## 🚀 快速开始（第一次使用）

如果你是第一次接触这个项目，请按照以下步骤配置你的开发环境。

### 1️⃣ 安装必需软件

#### 1.1 Visual Studio 2022
- **下载地址**：[Visual Studio 官网](https://visualstudio.microsoft.com/zh-hans/downloads/)
- **安装时请勾选以下工作负载**：
  - 使用 C++ 的桌面开发
  - Windows 10/11 SDK
- **安装教程**：[👉 点击查看教程链接](https://zhuanlan.zhihu.com/p/597927513)

#### 1.2 Git Bash
- **下载地址**：[Git 官网](https://git-scm.com/download/win)
- **安装时保持默认选项即可**。
- **安装教程**：[👉 点击查看教程链接](https://blog.csdn.net/Little_Carter/article/details/155110165)(可以不配置SSH)

#### 1.3 Clash for Windows（用于访问 GitHub 等网站）


### 2️⃣ 第一次使用前的配置

#### 2.1 安装 OpenCV
1. 安装 OpenCV 4.9.0 ：
- **安装教程**：[👉 点击查看教程链接](https://blog.csdn.net/Little_Carter/article/details/155110165)(可以不配置SSH)
2. 运行安装程序，将 OpenCV 解压到一个**没有空格和中文的路径**，例如 `D:\SDK\opencv`。
3. 安装后，你需要记住这个路径，稍后用来设置环境变量。

#### 2.2 安装 IDS 相机 SDK（uEye）
1. 从https://pan.sjtu.edu.cn/web/share/22587d65abf4806afae8b996042109a6下载并解压。
2. 安装后，你需要记住这个路径，稍后用来设置环境变量。

#### 2.3 设置系统环境变量
为了让 Visual Studio 能够找到 OpenCV 和 IDS 的库文件，我们需要添加两个环境变量：
<!-- 1. 打开设置，搜素环境变量
2. 点击“编辑系统环境变量” -> “环境变量”
3. 在下方“系统变量”中增加下面两个变量： -->

- **变量名**：`OPENCV_DIR`  
  **变量值**：你的 OpenCV 安装路径下的 `build` 文件夹，例如 `D:\opencv_490\opencv`

- **变量名**：`IDS_DIR`  
  **变量值**：你的 IDS SDK 安装路径，例如 `D:\ids-software-suite-win-4.96.1`

**设置方法**：
1. 右键“此电脑” → “属性” → “高级系统设置” → “环境变量”。
2. 在“系统变量”或“用户变量”中点击“新建”，输入变量名和变量值。
3. 确定保存后，**重启电脑**或重启 Visual Studio 使变量生效。

> ✅ 设置完成后，打开 Visual Studio 项目时，编译器会自动通过 `$(OPENCV_DIR)` 和 `$(IDS_DIR)` 找到依赖库。

#### 2.4 克隆代码仓库
打开 Git Bash，进入你想存放项目的文件夹，然后运行：
```bash
git clone https://github.com/ncqfs/transfer_stage_controll.git
cd transfer_stage_controll
```
现在你应该可以在文件夹中看到所有源代码了。

## 🔁 后续每次工作要做的事情

### 1. 打开 Git Bash，进入项目目录
```bash
cd /d/你的项目路径/transfer_stage_controll
```

### 2. 拉取最新代码（避免冲突）
```bash
git checkout dev          # 切换到开发分支
git pull origin dev       # 拉取远程最新代码
```

### 3. 创建自己的功能分支（以你的名字或功能命名）
每个新功能或修复都应该在独立的分支上进行，不要直接在 `dev` 上修改。
```bash
git checkout -b feature/你的功能名
```
例如：`git checkout -b feature/add-scan`  

### 4. 在 Visual Studio 中打开解决方案文件
双击 `transfer_stage_controll.sln`，开始编写代码。

### 5. 写完代码后，提交到本地仓库
```bash
git add .                 # 添加所有修改
git commit -m "简单描述你做了什么"
```

### 6. 推送到远程仓库（备份）
```bash
git push origin feature/你的功能名
```
如果这是第一次推送这个分支，需要加上 `-u` 选项建立跟踪关系：
```bash
git push -u origin feature/你的功能名
```

### 7. 当功能完成后，合并到 `dev` 分支
- 先切换到 `dev` 分支并拉取最新代码：
  ```bash
  git checkout dev
  git pull origin dev
  ```
- 将你的功能分支合并进来：
  ```bash
  git merge feature/你的功能名
  ```
- 如果有冲突，手动解决后提交（解决后 `git add` 再 `git commit`）。
- 最后推送到远程 `dev`：
  ```bash
  git push origin dev
  ```

### 8. 清理已合并的分支（可选）
```bash
git branch -d feature/你的功能名          # 删除本地分支
git push origin --delete feature/你的功能名  # 删除远程分支
```

---

## 📚 Git 常用命令速查

| 命令 | 说明 |
|------|------|
| `git clone <仓库地址>` | 将远程仓库下载到本地 |
| `git status` | 查看当前文件状态（哪些修改了、哪些未跟踪） |
| `git add .` | 添加所有修改到暂存区 |
| `git commit -m "信息"` | 提交暂存区的内容到本地仓库 |
| `git push` | 将本地提交推送到远程仓库（当前分支已关联时） |
| `git pull` | 从远程拉取最新代码并合并到当前分支 |
| `git branch` | 查看本地分支（当前分支前有 `*`） |
| `git branch -a` | 查看所有分支（包括远程） |
| `git checkout <分支名>` | 切换到指定分支 |
| `git checkout -b <新分支名>` | 创建并切换到新分支 |
| `git merge <分支名>` | 将指定分支合并到当前分支 |
| `git log --oneline` | 查看简洁的提交历史 |
| `git diff` | 查看工作区与暂存区的差异 |
| `git remote -v` | 查看远程仓库地址 |
| `git fetch` | 从远程获取最新数据（但不自动合并） |
| `git branch -d <分支名>` | 删除本地分支（已合并的） |
| `git push origin --delete <分支名>` | 删除远程分支 |

---

## 👥 第一次使用：联系仓库管理员加入项目

如果你是第一次参与本项目，需要先获得访问权限：

1. **联系项目负责人**（例如在微信群或私聊），告知你的 GitHub 用户名（注册 GitHub 时使用的账号）。
2. 负责人会邀请你成为仓库的 collaborator（合作者）。你会在注册邮箱或 GitHub 通知中收到邀请邮件，点击“接受邀请”即可。
3. 接受邀请后，你就有权限向仓库推送代码了。

---



如果问题仍然无法解决，可以在群里 @ 项目负责人或查看项目文档。

祝你开发顺利！🎉