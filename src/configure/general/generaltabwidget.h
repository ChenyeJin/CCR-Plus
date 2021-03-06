#ifndef GENERALCONFIGUREWIDGET_H
#define GENERALCONFIGUREWIDGET_H

#include "configure/configuretabwidget.h"

namespace Ui
{
class GeneralTabWidget;
}

class GeneralTabWidget : public ConfigureTabWidget
{
    Q_OBJECT
public:
    explicit GeneralTabWidget(QWidget* parent = nullptr);
    ~GeneralTabWidget();

    /// 载入 problem，显示具体配置
    virtual void ShowProblemConfiguration(Problem* problem) override;
    /// 转换题目类型
    virtual void ChangeProblemType(Global::ProblemType type) override;
    /// 重置为默认
    virtual void Reset() override;

    /// 焦点移到错误的 lineEdit
    void FocusErrorLine();

private:
    Ui::GeneralTabWidget* ui;
    bool load_finished;

private slots:
    void on_radioButton_builtin_clicked();
    void on_radioButton_custom_clicked();
    void on_pushButton_resetSubmit_clicked();
    void on_pushButton_resetRun_clicked();
    void on_pushButton_resetChecker_clicked();
    void on_lineEdit_dir_textChanged(const QString& text);
    void on_spinBox_codeLim_valueChanged(double val);
    void on_lineEdit_exe_textChanged(const QString& text);
    void on_lineEdit_inFile_textChanged(const QString& text);
    void on_lineEdit_outFile_textChanged(const QString& text);
    void on_comboBox_builtin_currentIndexChanged(const QString& text);
    void on_comboBox_custom_currentIndexChanged(const QString& text);
    void on_spinBox_checkerTimeLim_valueChanged(int val);
};

#endif // GENERALCONFIGUREWIDGET_H
