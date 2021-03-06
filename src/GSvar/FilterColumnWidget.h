#ifndef FILTERCOLUMNWIDGET_H
#define FILTERCOLUMNWIDGET_H

#include <QWidget>

namespace Ui {
class FilterColumnWidget;
}


class FilterColumnWidget
	: public QWidget
{
	Q_OBJECT

public:
	explicit FilterColumnWidget(QString name, QString tooltip);
	~FilterColumnWidget();

	enum State
	{
		NONE,
		KEEP,
		REMOVE
	};
	State state() const;
	void setState(State state);

    bool filter() const;
    void setFilter(bool checked);

signals:
    //any of the three checkboxes changed
	void stateChanged();

private slots:
    void keepClicked(bool checked);
    void removeClicked(bool checked);

private:
	Ui::FilterColumnWidget *ui;
};

#endif // FILTERCOLUMNWIDGET_H
