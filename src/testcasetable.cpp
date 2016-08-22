#include "testcasetable.h"

#include <QHeaderView>

using namespace std;

TestCaseTable::TestCaseTable(QWidget* parent) :
    QTableWidget(parent), problem(nullptr), sum_score(0)
{
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setSelectionMode(QAbstractItemView::ContiguousSelection);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setAlternatingRowColors(true);
    this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setStyleSheet(QLatin1String(
                            "QHeaderView"
                            "{"
                            "  background:#FFFFFF;"
                            "}"
                            "QTableWidget::item:alternate:!selected"
                            "{"
                            "  background-color:#FFFFFF;"
                            "}"
                            "QTableWidget::item:!alternate:!selected"
                            "{"
                            "  background-color:#F8F8F8;"
                            "}"));

    this->horizontalHeader()->setDefaultSectionSize(70);
    this->horizontalHeader()->setMinimumSectionSize(70);
    this->horizontalHeader()->setFixedHeight(22);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->horizontalHeader()->setHighlightSections(false);

    this->verticalHeader()->setDefaultSectionSize(22);
    this->verticalHeader()->setMinimumWidth(22);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->verticalHeader()->setHighlightSections(false);

    connect(this, &QTableWidget::itemSelectionChanged, this, &TestCaseTable::onItemSelectionChanged);
    connect(this, &QTableWidget::currentItemChanged, this, [this]()
    {
        int top, bottom;
        SelectionType type = GetSelectionType(&top, &bottom);
        if (type != NoSelection && ScoreItemBottomRow(bottom) == bottom && unselect_score_item)
            unselect_score_item->setFlags(unselect_score_item->flags() | Qt::ItemIsSelectable);
    });
}

void TestCaseTable::LoadTestCases(Problem *problem)
{
    score_item.clear();

    this->clear();
    this->setRowCount(0);
    this->setColumnCount(0);
    this->problem = problem;
    this->sum_score = problem->Score();
    this->unselect_score_item = nullptr;

    if (problem == nullptr) return;
    if (problem->Type() == Global::Traditional)
    {
        this->setColumnCount(5);
        this->setHorizontalHeaderLabels({"分值", "输入文件", "输出文件", "时间限制", "内存限制"});
        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
        this->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
        this->setColumnWidth(0, 70);
        this->setColumnWidth(3, 70);
        this->setColumnWidth(4, 70);
    }
    else if (problem->Type() == Global::AnswersOnly)
    {
        this->setColumnCount(4);
        this->setHorizontalHeaderLabels({"分值", "输入文件", "输出文件", "提交文件"});
        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        this->setColumnWidth(0, 70);
    }
    else
        return;

    int rows = 0;
    for (int i = 0; i < problem->SubtaskCount(); i++)
    {
        Subtask* sub = problem->SubtaskAt(i);
        int len = 0;
        for (TestCase* point : *sub)
        {
            this->insertRow(rows);

            addItem(rows, 1, point->InFile());
            addItem(rows, 2, point->OutFile());

            if (problem->Type() == Global::Traditional)
            {
                addItem(rows, 3, QString::number(point->TimeLimit()));
                addItem(rows, 4, QString::number(point->MemoryLimit()));
            }
            else if (problem->Type() == Global::AnswersOnly)
                addItem(rows, 3, point->SubmitFile());

            addItem(rows, 0, QString::number(sub->Score()));
            this->item(rows, 0)->setBackgroundColor(QColor(255, 255, 255));

            len++, rows++;
            score_item.push_back(this->item(rows - len, 0));
        }
        if (len > 1) this->setSpan(rows - len, 0, len, 1);
    }
}

TestCaseTable::SelectionType TestCaseTable::GetSelectionType(int* _top, int* _bottom)
{
    auto list = this->selectedRanges();
    int top = 1e9, bottom = -1e9;
    for (auto i : list)
    {
        top = min(top, i.topRow());
        bottom = max(bottom, i.bottomRow());
    }
    *_top = top, *_bottom = bottom;
    if (top > bottom) return NoSelection;

    for (int i = top; i <= bottom; i++)
        if (!this->item(i, 1)->isSelected())
            return SelectDiscontinuous;

    if (top == ScoreItemTopRow(top) && bottom == ScoreItemBottomRow(bottom))
    {
        if (top == bottom)
            return SelectOneTestCasePackage;
        if (ScoreItemTopRow(bottom) == top)
            return SelectOnePackage;
        for (int i = top; i <= bottom; i++)
            if (ScoreItemTopRow(i) != ScoreItemBottomRow(i))
                return SelectMultiplePackage;
        return SelectMultipleTestCasePackage;
    }

    if (top == bottom) return SelectOneSubTestCase;
    if (ScoreItemAt(top) == ScoreItemAt(bottom)) return SelectMultipleSubTestCase;
    return OtherSelection;
}

void TestCaseTable::swapTestCase(int row1, int row2)
{
    for (int c = 1; c < this->columnCount(); c++)
    {
        QTableWidgetItem *item1 = this->takeItem(row1, c),
                         *item2 = this->takeItem(row2, c);
        this->setItem(row1, c, item2);
        this->setItem(row2, c, item1);
    }
}

void TestCaseTable::swapPackage(int topRow1, int topRow2)
{
    vector<QTableWidgetItem*> tmp;
    int p1 = topRow1, s1 = ScoreItemBottomRow(topRow1) - topRow1 + 1,
        p2 = topRow2, s2 = ScoreItemBottomRow(topRow2) - topRow2 + 1,
        p3 = p1 + s2;

    if (this->rowSpan(p1, 0) > 1) this->setSpan(p1, 0, 1, 1);
    if (this->rowSpan(p2, 0) > 1) this->setSpan(p2, 0, 1, 1);
    for (int c = 0; c < this->columnCount(); c++)
    {
        tmp.clear();
        for (int i = 0; i < s1; i++) tmp.push_back(this->takeItem(p1 + i, c));
        for (int i = 0; i < s2; i++) this->setItem(p1 + i, c, this->takeItem(p2 + i, c));
        for (int i = 0; i < s1; i++) this->setItem(p3 + i, c, tmp[i]);
    }
    for (int i = 0; i < s2; i++) score_item[p1 + i] = this->item(p1, 0);
    for (int i = 0; i < s1; i++) score_item[p3 + i] = this->item(p3, 0);
    if (s2 > 1) this->setSpan(p1, 0, s2, 1);
    if (s1 > 1) this->setSpan(p3, 0, s1, 1);
}



void TestCaseTable::AddTestCase(TestCase* point, int score)
{
    int top, bottom, row;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != NoSelection && type != SelectOnePackage && type != SelectOneTestCasePackage) return;

    if (type == NoSelection) bottom = -1;
    this->insertRow(row = bottom + 1);

    addItem(row, 1, point->InFile());
    addItem(row, 2, point->OutFile());

    if (problem->Type() == Global::Traditional)
    {
        addItem(row, 3, QString::number(point->TimeLimit()));
        addItem(row, 4, QString::number(point->MemoryLimit()));
    }
    else if (problem->Type() == Global::AnswersOnly)
        addItem(row, 3, point->SubmitFile());

    addItem(row, 0, QString::number(score));
    score_item.insert(score_item.begin() + row, this->item(row, 0));
    sum_score += score;
    this->selectRow(row);
    if (!this->hasFocus()) this->setFocus();
}

void TestCaseTable::AddSubTestCase(TestCase* point)
{
    int top, bottom, row;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectOnePackage && type != SelectOneSubTestCase && type != SelectOneTestCasePackage) return;

    int span = this->rowSpan(ScoreItemTopRow(top), 0) + 1;
    this->insertRow(row = bottom + 1);

    addItem(row, 1, point->InFile());
    addItem(row, 2, point->OutFile());

    if (problem->Type() == Global::Traditional)
    {
        addItem(row, 3, QString::number(point->TimeLimit()));
        addItem(row, 4, QString::number(point->MemoryLimit()));
    }
    else if (problem->Type() == Global::AnswersOnly)
        addItem(row, 3, point->SubmitFile());

    addItem(row, 0, "");
    this->setSpan(ScoreItemTopRow(top), 0, span, 1);
    score_item.insert(score_item.begin() + row, score_item[top]);
    this->selectRow(row);
}

void TestCaseTable::RemoveSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type == NoSelection || type == SelectDiscontinuous) return;

    this->clearSelection();
    for (int r = top; r <= bottom; r++)
    {
        if (ScoreItemTopRow(r) == r && ScoreItemBottomRow(r) > bottom)
        {
            int scoreBottom = ScoreItemBottomRow(r);
            QTableWidgetItem* item = this->item(bottom + 1, 0);
            item->setText(ScoreItemAt(r)->text());
            item->setToolTip(item->text());
            for (int i = bottom + 1; i <= scoreBottom; i++) score_item[i] = item;
        }
        else if (ScoreItemTopRow(r) == r)
            sum_score -= this->item(r, 0)->text().toInt();
        for (int c = 0; c < this->columnCount(); c++) this->takeItem(r, c);
    }

    for (int r = bottom + 1; r < this->rowCount(); r++)
    {
        int p = r - bottom - 1 + top;
        for (int c = 0; c < this->columnCount(); c++) this->setItem(p, c, this->takeItem(r, c));
    }

    for (int r = 0; r < this->rowCount(); r++)
        if (this->rowSpan(r, 0) > 1) this->setSpan(r, 0, 1, 1);

    for (int i = top; i <= bottom; i++) this->removeRow(this->rowCount() - 1);
    score_item.erase(score_item.begin() + top, score_item.begin() + bottom + 1);

    for (int i = 0, j; i < this->rowCount(); i++) if (ScoreItemTopRow(i) == i)
    {
        for (j = i; j < this->rowCount() && score_item[j] == score_item[i]; j++);
        if (j - i > 1) this->setSpan(i, 0, j - i, 1);
    }
}

void TestCaseTable::MoveUpSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    this->clearSelection();
    if (type == SelectOneSubTestCase)
    {
        swapTestCase(top - 1, top);
        this->selectRow(top - 1);
    }
    else if (type == SelectOnePackage || type == SelectOneTestCasePackage)
    {
        int p = ScoreItemTopRow(top - 1);
        swapPackage(ScoreItemTopRow(top - 1), top);
        QTableWidgetSelectionRange range(p, 0, ScoreItemBottomRow(p), this->columnCount() - 1);
        this->setRangeSelected(range, true);
    }
}

void TestCaseTable::MoveDownSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    this->clearSelection();
    if (type == SelectOneSubTestCase)
    {
        swapTestCase(top + 1, top);
        this->selectRow(top + 1);
    }
    else if (type == SelectOnePackage || type == SelectOneTestCasePackage)
    {
        swapPackage(top, ScoreItemBottomRow(top) + 1);
        int p = ScoreItemBottomRow(top) + 1;
        QTableWidgetSelectionRange range(p, 0, ScoreItemBottomRow(p), this->columnCount() - 1);
        this->setRangeSelected(range, true);
    }
}

void TestCaseTable::MergeSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectMultiplePackage && type != SelectMultipleTestCasePackage) return;

    int sum = 0;
    QTableWidgetItem* scoreItem = score_item[top];
    for (int i = top; i <= bottom; i++)
    {
        if (ScoreItemTopRow(i) == i)
        {
            sum += score_item[i]->text().toInt();
            if (this->rowSpan(i, 0) > 1) this->setSpan(i, 0, 1, 1);
        }
        score_item[i] = scoreItem;
    }
    this->setSpan(top, 0, bottom - top + 1, 1);
    scoreItem->setText(QString::number(sum));
    scoreItem->setToolTip(scoreItem->text());
    onItemSelectionChanged();
}

void TestCaseTable::SplitSelection()
{
    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);
    if (type != SelectOnePackage && type != SelectMultiplePackage) return;

    vector<int> score;
    for (int i = top; i <= bottom; i++)
    {
        if (ScoreItemTopRow(i) == i && this->rowSpan(i, 0) > 1)
        {
            int len = this->rowSpan(i, 0);
            int sum = score_item[i]->text().toInt(), s = sum / len;
            this->setSpan(i, 0, 1, 1);
            score.clear();
            for (int j = 0; j < len; j++)
            {
                score_item[i + j] = this->item(i + j, 0);
                score.push_back(s), sum -= s;
            }
            for (int j = len; sum; sum--) score[--j]++;
            for (int j = 0; j < len; j++)
            {
                score_item[i + j]->setText(QString::number(score[j]));
                score_item[i + j]->setToolTip(QString::number(score[j]));
            }
        }
    }
    onItemSelectionChanged();
}

void TestCaseTable::onItemSelectionChanged()
{
    can_add = can_add_sub = can_remove = can_up = can_down = can_merge = can_split = false;

    int top, bottom;
    SelectionType type = GetSelectionType(&top, &bottom);

    if (type == NoSelection)
    {
        can_add = true;
        emit testCaseSelectionChanged();
        return;
    }
    if (type == SelectDiscontinuous)
    {
        emit testCaseSelectionChanged();
        return;
    }
    if (ScoreItemBottomRow(bottom) > bottom)
    {
        if (this->ScoreItemAt(bottom)->isSelected())
        {
            unselect_score_item = this->ScoreItemAt(bottom);
            unselect_score_item->setFlags(unselect_score_item->flags() ^ Qt::ItemIsSelectable);
        }
    }
    else
        this->ScoreItemAt(bottom)->setFlags(this->ScoreItemAt(bottom)->flags() | Qt::ItemIsSelectable);

    can_remove = true;
    if (type == SelectOnePackage || type == SelectOneTestCasePackage) can_add = true;
    if (type == SelectMultiplePackage || type == SelectMultipleTestCasePackage) can_merge = true;
    if (type == SelectOnePackage || type == SelectMultiplePackage) can_split = true;
    if (type == SelectOnePackage || type == SelectOneSubTestCase || type == SelectOneTestCasePackage)
    {
        can_add_sub = true;
        if (type == SelectOneSubTestCase)
        {
            if (ScoreItemTopRow(top) < top) can_up = true;
            if (ScoreItemBottomRow(bottom) > bottom) can_down = true;
        }
        else
        {
            if (!this->item(0, 1)->isSelected()) can_up = true;
            if (!this->item(this->rowCount() - 1, 1)->isSelected()) can_down = true;
        }
    }
    emit testCaseSelectionChanged();
}
