# proxy配置

## 本地

华为云，编辑安全组，添加入方向规则，7891
https://bbs.huaweicloud.com/blogs/355312

端口映射
https://support.huaweicloud.com/ecs_faq/ecs_faq_1328.html

在 `C:\Users\${YOURNAME}\.ssh\config` 里面改变config

```ssh config
Host ${公网IP}
  HostName ${公网IP}
  RemoteForward 7891 127.0.0.1:7890
  User lab
```

## 远程

改变远程 `~/.bashrc`

```bash
# Source default setting
[ -f /etc/bashrc ] && . /etc/bashrc

# User environment PATH
export PATH

PORT=7891
export HTTP_PROXY=http://localhost:$PORT
export HTTPS_PROXY=http://localhost:$PORT
export http_proxy=http://localhost:$PORT
export https_proxy=http://localhost:$PORT
```

# 前期配置

## glfw

在华为云远程服务器里面compile glfw

```bash
cd downloads
git clone https://github.com/glfw/glfw.git
cd glfw
git checkout tags/3.4

# 安装依赖
sudo yum install doxygen
sudo dnf install cmake wayland-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel

# 准备编译
cmake -B build
cd build

# 编译
make -j4
sudo make install
```

## OpenGL

说实话，我不知道是哪一个起了效果，总之它起效果了

```bash
sudo dnf install xorg-x11-server-Xvfb libXcursor libXrandr libXinerama libXi
sudo yum install mesa-libGL-devel mesa-libGLU-devel mesa-demos
sudo yum install mesa-libGL mesa-libGLES mesa-dri-drivers
sudo yum install xorg-x11-server
```

因为glfw不支持headless运行，因此需要一个虚拟的xserver在运行
在 `.bashrc` 里面加入这几行

```bash
export XDG_RUNTIME_DIR=/run/user/$(id -u)

if [ ! -d "$XDG_RUNTIME_DIR" ]; then
    sudo mkdir -p "$XDG_RUNTIME_DIR"
    sudo chmod 0700 "$XDG_RUNTIME_DIR"
fi

pkill -f "Xvfb"
Xvfb :99 -screen 0 1920x1080x24 +extension GLX &
export DISPLAY=:99
```

## 编译

```bash
cd k_means_bvh
git pull
cmake -B build
cmake --build build
```

## 运行

为了方便，使用脚本启动 `run.sh`

```bash
#!/bin/bash

cmake --build build

# copy
cp build/BVHVisualization ./BVHVisualization

# select model, if no arg then the model is Cow
DEFAULT_MODEL="Cow"
MODEL="$DEFAULT_MODEL"

# select model by arg
if [ $# -ge 1 ]; then
    case "$1" in
        Dragon|Cow|Face|Car)
            MODEL="$1"
            ;;
        *)
            echo "error：invalid arg '$1'"
            echo "avaliable: Dragon, Cow, Face, Car"
            exit 1
            ;;
    esac
fi


./BVHVisualization --no_gui "$MODEL"
```

这样运行
```bash
chmod +x run.sh
./run.sh
```

目前只有四个模型可选

```bash
./run.sh Dragon
./run.sh Cow
./run.sh Face
./run.sh Car
```