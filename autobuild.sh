#!/bin/bash
set -e

# 获取当前项目根目录
BASE_DIR=$(pwd)

# 如果没有build目录，创建该目录
if [ ! -d "$BASE_DIR/build" ]; then
    mkdir "$BASE_DIR/build"
fi

# 清空编译缓存
rm -rf "$BASE_DIR/build/*"

# 编译
cd "$BASE_DIR/build"
cmake ..
make

# 回到根目录
cd "$BASE_DIR"

# ====================== 安装头文件 ======================
echo "安装头文件到 /usr/include/my_muduo..."

if [ ! -d /usr/include/my_muduo ]; then
    sudo mkdir -p /usr/include/my_muduo
fi

# 拷贝项目 include 目录下的所有头文件
for header in "$BASE_DIR"/include/*.h; do
    sudo cp "$header" /usr/include/my_muduo/
done

# ====================== 安装动态库 ======================
echo "安装动态库到 /usr/lib..."
sudo cp "$BASE_DIR/lib/libmy_muduo.so" /usr/lib/

# 刷新系统库缓存
sudo ldconfig

echo "========================================"
echo " ✅ my_muduo 安装成功！"
echo "========================================"