//
// Created by 20852 on 25-7-22.
//
#include <QApplication> // QApplication 所需
#include <QIcon> // 需要包含 QIcon 头文件
#include "login_in.h"   // 创建 LoginWindow 对象所需

int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // 创建应用程序对象

    app.setWindowIcon(QIcon(":/icons/Icon/114514.png")); // 设置图标

    LoginWindow loginWindow; // 创建登录窗口的实例
    loginWindow.show();      // 显示登录窗口

    return app.exec(); // 启动 Qt 事件循环
}