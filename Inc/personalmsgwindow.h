//
// Created by 20852 on 2025/8/20.
//

#ifndef PERSONALMSGWINDOW_H
#define PERSONALMSGWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class PersonalMsgWindow : public QDialog {
    Q_OBJECT

public:
    explicit PersonalMsgWindow(QWidget *parent = nullptr);
    ~PersonalMsgWindow();

private:
    void setupUI();
};

#endif // PERSONALMSGWINDOW_H