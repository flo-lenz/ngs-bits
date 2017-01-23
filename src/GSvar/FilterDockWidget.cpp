#include "FilterDockWidget.h"
#include "Settings.h"
#include "Helper.h"
#include "FilterColumnWidget.h"
#include "NGSD.h"
#include "Log.h"
#include <QCheckBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QCompleter>

FilterDockWidget::FilterDockWidget(QWidget *parent)
	: QDockWidget(parent)
	, ui_()
{
	ui_.setupUi(this);

	connect(ui_.maf, SIGNAL(valueChanged(double)), this, SIGNAL(filtersChanged()));
	connect(ui_.maf_enabled, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));

	connect(ui_.impact, SIGNAL(currentIndexChanged(int)), this, SIGNAL(filtersChanged()));
	connect(ui_.impact_enabled, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));

	connect(ui_.ihdb, SIGNAL(valueChanged(int)), this, SIGNAL(filtersChanged()));
	connect(ui_.ihdb_enabled, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));
	connect(ui_.ihdb_ignore_gt, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));

	connect(ui_.classification, SIGNAL(currentIndexChanged(int)), this, SIGNAL(filtersChanged()));
	connect(ui_.classification_enabled, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));

	connect(ui_.geno, SIGNAL(currentIndexChanged(int)), this, SIGNAL(filtersChanged()));
	connect(ui_.geno_enabled, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));

	connect(ui_.keep_class_ge_enabled, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));
	connect(ui_.keep_class_ge, SIGNAL(currentTextChanged(QString)), this, SIGNAL(filtersChanged()));
	connect(ui_.keep_class_m, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));

	connect(ui_.compound_enabled, SIGNAL(toggled(bool)), this, SIGNAL(filtersChanged()));

	connect(ui_.roi_add, SIGNAL(clicked()), this, SLOT(addRoi()));
	connect(ui_.roi_add_temp, SIGNAL(clicked()), this, SLOT(addRoiTemp()));
	connect(ui_.roi_remove, SIGNAL(clicked()), this, SLOT(removeRoi()));
	connect(ui_.rois, SIGNAL(currentIndexChanged(int)), this, SLOT(roiSelectionChanged(int)));

	connect(ui_.ref_add, SIGNAL(clicked()), this, SLOT(addRef()));
	connect(ui_.ref_remove, SIGNAL(clicked()), this, SLOT(removeRef()));
	connect(ui_.refs, SIGNAL(currentIndexChanged(int)), this, SLOT(referenceSampleChanged(int)));

	connect(ui_.gene, SIGNAL(editingFinished()), this, SLOT(geneChanged()));
	connect(ui_.region, SIGNAL(editingFinished()), this, SLOT(regionChanged()));

	loadTargetRegions();
	loadReferenceFiles();

	reset(true, true);
}

void FilterDockWidget::setFilterColumns(const QMap<QString, QString>& filter_cols)
{
	//remove old widgets
	QLayoutItem* item;
	while ((item = ui_.filter_col->layout()->takeAt(0)) != nullptr)
	{
		delete item->widget();
		delete item;
	}

	//add new widgets
	auto it = filter_cols.cbegin();
	while(it!=filter_cols.cend())
	{
		auto w = new FilterColumnWidget(it.key(), it.value());
		connect(w, SIGNAL(stateChanged()), this, SLOT(filterColumnStateChanged()));
		ui_.filter_col->layout()->addWidget(w);

		++it;
	}
	ui_.filter_col->layout()->addItem(new QSpacerItem(1,1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
}

void FilterDockWidget::loadTargetRegions()
{
	ui_.rois->blockSignals(true);

	//store old selection
	QString current = ui_.rois->currentText();

	ui_.rois->clear();
	ui_.rois->addItem("", "");
	ui_.rois->addItem("none", "");
	ui_.rois->insertSeparator(ui_.rois->count());

	//load ROIs of NGSD processing systems
	try
	{
		QMap<QString, QString> systems = NGSD().getProcessingSystems(true, true);
		auto it = systems.constBegin();
		while (it != systems.constEnd())
		{
			ui_.rois->addItem("Processing system: " + it.key(), Helper::canonicalPath(it.value()));
			++it;
		}
		ui_.rois->insertSeparator(ui_.rois->count());
	}
	catch (Exception& e)
	{
		Log::warn("Could not load NGSD processing system target regions: " + e.message());
	}

	//load ROIs of sub-panels
	try
	{
		QStringList subpanels = Helper::findFiles(NGSD::getTargetFilePath(true), "*.bed", false);
		subpanels.sort(Qt::CaseInsensitive);
		foreach(QString file, subpanels)
		{
			QString name = QFileInfo(file).fileName();
			name = name.left(name.size()-4);
			ui_.rois->addItem("Sub-panel: " + name, Helper::canonicalPath(file));
		}
		ui_.rois->insertSeparator(ui_.rois->count());
	}
	catch (Exception& e)
	{
		Log::warn("Could not load sub-panels target regions: " + e.message());
	}

	//load additional ROIs from settings
	QStringList rois = Settings::stringList("target_regions");
	std::sort(rois.begin(), rois.end(), [](const QString& a, const QString& b){return QFileInfo(a).fileName().toUpper() < QFileInfo(b).fileName().toUpper();});
	foreach(const QString& roi_file, rois)
	{
		QFileInfo info(roi_file);
		ui_.rois->addItem(info.fileName(), roi_file);
	}

	//restore old selection
	int current_index = ui_.rois->findText(current);
	if (current_index==-1) current_index = 1;
	ui_.rois->setCurrentIndex(current_index);

	ui_.rois->blockSignals(false);
}

void FilterDockWidget::loadReferenceFiles()
{
	//store old selection
	QString current = ui_.rois->currentText();

	//load from settings
	ui_.refs->clear();
	ui_.refs->addItem("none", "");
	QStringList refs = Settings::stringList("reference_files");
	foreach(const QString& roi_file, refs)
	{
		QStringList parts = roi_file.trimmed().split("\t");
		if (parts.count()!=2) continue;
		ui_.refs->addItem(parts[0], Helper::canonicalPath(parts[1]));
	}

	//restore old selection
	int current_index = ui_.refs->findText(current);
	if (current_index==-1) current_index = 0;
    ui_.refs->setCurrentIndex(current_index);
}

void FilterDockWidget::resetSignalsUnblocked(bool clear_roi, bool clear_off_target)
{
    //annotations
    ui_.maf_enabled->setChecked(false);
    ui_.maf->setValue(1.0);
    ui_.impact_enabled->setChecked(false);
    ui_.impact->setCurrentText("HIGH,MODERATE,LOW");
    ui_.ihdb_enabled->setChecked(false);
	ui_.ihdb->setValue(20);
	ui_.ihdb_ignore_gt->setChecked(false);
	ui_.classification_enabled->setChecked(false);
    ui_.classification->setCurrentText("3");
    ui_.geno_enabled->setChecked(false);
    ui_.geno->setCurrentText("hom");
    ui_.keep_class_ge_enabled->setChecked(false);
    ui_.keep_class_ge->setCurrentText("3");
	ui_.keep_class_m->setChecked(false);
    ui_.compound_enabled->setChecked(false);

    //filter cols
    QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
    foreach(FilterColumnWidget* w, fcws)
    {
        w->setState(FilterColumnWidget::NONE);
        w->setFilter(false);

		//disable off-target by default
		if (w->objectName()=="off-target")
		{
			if (clear_off_target)
			{
				w->setState(FilterColumnWidget::NONE);
			}
			else
			{
				w->setState(FilterColumnWidget::REMOVE);
			}
		}
    }

    //rois
	if (clear_roi)
	{
		ui_.rois->setCurrentIndex(1);
		ui_.rois->setToolTip("");
	}

    //gene
    last_genes_.clear();
    ui_.gene->clear();
    ui_.region->clear();

    //refs
    ui_.refs->setCurrentIndex(0);
    ui_.refs->setToolTip("");
}

void FilterDockWidget::reset(bool clear_roi, bool clear_off_target)
{
	blockSignals(true);
	resetSignalsUnblocked(clear_roi, clear_off_target);
	blockSignals(false);

    emit filtersChanged();
}

void FilterDockWidget::applyDefaultFilters()
{
	//block signals to avoid 10 updates of GUI
	blockSignals(true);

	resetSignalsUnblocked(false, true);

	//enable default filters
	ui_.maf_enabled->setChecked(true);
	ui_.maf->setValue(1.0);
	ui_.impact_enabled->setChecked(true);
	ui_.impact->setCurrentText("HIGH,MODERATE,LOW");
	ui_.ihdb_enabled->setChecked(true);
	ui_.ihdb->setValue(20);
	ui_.ihdb_ignore_gt->setChecked(false);
	ui_.classification_enabled->setChecked(true);
	ui_.classification->setCurrentText("3");
    ui_.keep_class_ge_enabled->setChecked(true);
	ui_.keep_class_ge->setCurrentText("3");
	ui_.keep_class_m->setChecked(true);

	//filter cols
	QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
	foreach(FilterColumnWidget* w, fcws)
	{
		if (w->objectName()=="anno_high_impact" || w->objectName()=="anno_pathogenic_clinvar" || w->objectName()=="anno_pathogenic_hgmd")
		{
			w->setState(FilterColumnWidget::KEEP);
		}
		else if (w->objectName()=="off-target")
		{
			w->setState(FilterColumnWidget::REMOVE);
		}
	}

	//re-enable signals
	blockSignals(false);

	//emit signal to update GUI
    emit filtersChanged();
}

void FilterDockWidget::applyDefaultFiltersTrio()
{
	//block signals to avoid 10 updates of GUI
	blockSignals(true);

	resetSignalsUnblocked(false, true);

	//enable default filters
	ui_.maf_enabled->setChecked(true);
	ui_.maf->setValue(1.0);
	ui_.impact_enabled->setChecked(true);
	ui_.impact->setCurrentText("HIGH,MODERATE");
	ui_.ihdb_enabled->setChecked(true);
	ui_.ihdb->setValue(20);
	ui_.ihdb_ignore_gt->setChecked(false);
	ui_.keep_class_ge_enabled->setChecked(true);
	ui_.keep_class_ge->setCurrentText("4");

	//filter cols
	QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
	foreach(FilterColumnWidget* w, fcws)
	{
		if (w->objectName().startsWith("trio_"))
		{
			w->setState(FilterColumnWidget::KEEP);
			w->setFilter(true);
		}
	}

	//re-enable signals
	blockSignals(false);

	//emit signal to update GUI
	emit filtersChanged();
}

void FilterDockWidget::applyDefaultFiltersMultiSample()
{
	//block signals to avoid 10 updates of GUI
	blockSignals(true);

	resetSignalsUnblocked(false, true);

	//enable default filters
	ui_.maf_enabled->setChecked(true);
	ui_.maf->setValue(1.0);
	ui_.impact_enabled->setChecked(true);
	ui_.impact->setCurrentText("HIGH,MODERATE,LOW");
	ui_.ihdb_enabled->setChecked(true);
	ui_.ihdb->setValue(20);
	ui_.ihdb_ignore_gt->setChecked(true);
	ui_.classification_enabled->setChecked(true);
	ui_.classification->setCurrentText("3");
	ui_.keep_class_ge_enabled->setChecked(true);
	ui_.keep_class_ge->setCurrentText("3");
	ui_.keep_class_m->setChecked(true);

	//filter cols
	QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
	foreach(FilterColumnWidget* w, fcws)
	{
		if (w->objectName()=="anno_high_impact" || w->objectName()=="anno_pathogenic_clinvar" || w->objectName()=="anno_pathogenic_hgmd")
		{
			w->setState(FilterColumnWidget::KEEP);
		}
		else if (w->objectName()=="off-target")
		{
			w->setState(FilterColumnWidget::REMOVE);
		}
	}

	//re-enable signals
	blockSignals(false);

	//emit signal to update GUI
	emit filtersChanged();
}

void FilterDockWidget::applyDefaultFiltersSomatic()
{
    //block signals to avoid 10 updates of GUI
    blockSignals(true);

	resetSignalsUnblocked(false, true);

    //enable default filters
    ui_.maf_enabled->setChecked(true);
    ui_.maf->setValue(1.0);

    //filter cols
    QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
    foreach(FilterColumnWidget* w, fcws)
	{
		w->setState(FilterColumnWidget::REMOVE);
	}

    //re-enable signals
    blockSignals(false);

    //emit signal to update GUI
    emit filtersChanged();
}

bool FilterDockWidget::applyMaf() const
{
	return ui_.maf_enabled->isChecked();
}

double FilterDockWidget::mafPerc() const
{
	return ui_.maf->value();
}

bool FilterDockWidget::applyImpact() const
{
	return ui_.impact_enabled->isChecked();
}

QStringList FilterDockWidget::impact() const
{
	return ui_.impact->currentText().split(",");
}

bool FilterDockWidget::applyClassification() const
{
	return ui_.classification_enabled->isChecked();
}

int FilterDockWidget::classification() const
{
	return ui_.classification->currentText().toInt();
}

bool FilterDockWidget::applyGenotype() const
{
	return ui_.geno_enabled->isChecked();
}

QString FilterDockWidget::genotype() const
{
	return ui_.geno->currentText();
}

bool FilterDockWidget::applyIhdb() const
{
	return ui_.ihdb_enabled->isChecked();
}

int FilterDockWidget::ihdb() const
{
	return ui_.ihdb->value();
}

int FilterDockWidget::ihdbIgnoreGenotype() const
{
	return ui_.ihdb_ignore_gt->isChecked();
}

bool FilterDockWidget::applyCompoundHet() const
{
	return ui_.compound_enabled->isChecked();
}

int FilterDockWidget::keepClassGreaterEqual() const
{
	if (!ui_.keep_class_ge_enabled->isChecked()) return -1;
	return ui_.keep_class_ge->currentText().toInt();
}

bool FilterDockWidget::keepClassM() const
{
	return ui_.keep_class_m->isChecked();
}

QStringList FilterDockWidget::filterColumnsKeep() const
{
	QStringList output;
	QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
	foreach(FilterColumnWidget* w, fcws)
	{
		if (w->state()==FilterColumnWidget::KEEP)
		{
			output.append(w->objectName());
		}
	}
	return output;
}

QStringList FilterDockWidget::filterColumnsRemove() const
{
	QStringList output;
	QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
	foreach(FilterColumnWidget* w, fcws)
	{
		if (w->state()==FilterColumnWidget::REMOVE)
		{
			output.append(w->objectName());
		}
	}
    return output;
}

QStringList FilterDockWidget::filterColumnsFilter() const
{
	QStringList output;
    QList<FilterColumnWidget*> fcws = ui_.filter_col->findChildren<FilterColumnWidget*>();
    foreach(FilterColumnWidget* w, fcws)
    {
        if (w->filter())
        {
			output.append(w->objectName());
        }
    }
    return output;
}

QString FilterDockWidget::targetRegion() const
{
	return ui_.rois->toolTip();
}

void FilterDockWidget::setTargetRegion(QString roi_file)
{
	roi_file = Helper::canonicalPath(roi_file);
	for (int i=0; i<ui_.rois->count(); ++i)
	{
		if (ui_.rois->itemData(i).toString()==roi_file)
		{
			ui_.rois->setCurrentIndex(i);
			break;
		}
	}
}

QStringList FilterDockWidget::genes() const
{
	QStringList genes = ui_.gene->text().split(',');

	for(int i=0; i<genes.count(); ++i)
	{
		 genes[i] = genes[i].trimmed().toUpper();
	}

	genes.removeAll("");
	genes.removeDuplicates();

	return genes;
}

QString FilterDockWidget::region() const
{
	return ui_.region->text().trimmed();
}

QString FilterDockWidget::referenceSample() const
{
	return ui_.refs->toolTip();
}

QMap<QString, QString> FilterDockWidget::appliedFilters() const
{
	QMap<QString, QString> output;
	if (applyMaf()) output.insert("maf", QString::number(mafPerc(), 'f', 2) + "%");
	if (applyImpact()) output.insert("impact", impact().join(","));
	if (applyIhdb()) output.insert("ihdb", QString::number(ihdb()));
	if (applyClassification()) output.insert("classification", QString::number(classification()));
	if (applyGenotype()) output.insert("genotype" , genotype());
	if (keepClassM()) output.insert("keep_class_m", "");
	if (keepClassGreaterEqual()!=-1) output.insert("keep_class_ge", QString::number(keepClassGreaterEqual()));

	return output;
}

void FilterDockWidget::addRoi()
{
	//get file to open
	QString path = Settings::path("path_regions");
	QString filename = QFileDialog::getOpenFileName(this, "Select target region file", path, "BED files (*.bed);;All files (*.*)");
	if (filename=="") return;

	//store open path
	Settings::setPath("path_regions", filename);

	//update settings
	QStringList rois = Settings::stringList("target_regions");
	rois.append(filename);
	rois.sort(Qt::CaseInsensitive);
	rois.removeDuplicates();
	Settings::setStringList("target_regions", rois);

	//update GUI
	loadTargetRegions();
}

void FilterDockWidget::addRoiTemp()
{
	//get file to open
	QString path = Settings::path("path_regions");
	QString filename = QFileDialog::getOpenFileName(this, "Select target region file", path, "BED files (*.bed);;All files (*.*)");
	if (filename=="") return;

	//add to list
	ui_.rois->addItem(QFileInfo(filename).fileName(), Helper::canonicalPath(filename));
}

void FilterDockWidget::removeRoi()
{
	QString filename = ui_.rois->itemData(ui_.rois->currentIndex()).toString();
	if (filename=="") return;

	//update settings
	QStringList rois = Settings::stringList("target_regions");
	rois.removeOne(filename);
	Settings::setStringList("target_regions", rois);

	//update GUI
	loadTargetRegions();
	emit filtersChanged();
}

void FilterDockWidget::roiSelectionChanged(int index)
{
	//delete old completer
	QCompleter* completer_old = ui_.rois->completer();
	if ((void*)completer_old!=0)
	{
		completer_old->deleteLater();
	}

	//create completer for search mode
	if (ui_.rois->currentIndex()==0)
	{
		ui_.rois->setEditable(true);

		QCompleter* completer = new QCompleter(ui_.rois->model(), ui_.rois);
		completer->setCompletionMode(QCompleter::PopupCompletion);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
		completer->setFilterMode(Qt::MatchContains);
		completer->setCompletionRole(Qt::DisplayRole);
		ui_.rois->setCompleter(completer);
	}
	else
	{
		ui_.rois->setEditable(false);
	}


	ui_.rois->setToolTip(ui_.rois->itemData(index).toString());

	if(index!=0)
	{
		emit filtersChanged();
	}
	//qDebug() << __LINE__ << ui_.rois->completer() << ui_.rois->itemData(index).toString();
}


void FilterDockWidget::referenceSampleChanged(int index)
{
	ui_.refs->setToolTip(ui_.refs->itemData(index).toString());
}

void FilterDockWidget::geneChanged()
{
	if (genes()!=last_genes_)
	{
		last_genes_ = genes();
		emit filtersChanged();
	}
}

void FilterDockWidget::regionChanged()
{
	emit filtersChanged();
}

void FilterDockWidget::filterColumnStateChanged()
{
	emit filtersChanged();
}

void FilterDockWidget::addRef()
{
	//get file to open
	QString path = Settings::path("path_variantlists");
	QString filename = QFileDialog::getOpenFileName(this, "Select reference file", path, "BAM files (*.bam);;All files (*.*)");
	if (filename=="") return;

	//get name
	QString name = QInputDialog::getText(this, "Reference file name", "Display name:");
	if (name=="") return;

	//update settings
	QStringList refs = Settings::stringList("reference_files");
	refs.append(name + "\t" + filename);
	refs.sort(Qt::CaseInsensitive);
	refs.removeDuplicates();
	Settings::setStringList("reference_files", refs);

	//update GUI
	loadReferenceFiles();
}

void FilterDockWidget::removeRef()
{
	QString name = ui_.refs->itemText(ui_.refs->currentIndex());
	QString filename = ui_.refs->itemData(ui_.refs->currentIndex()).toString();
	if (filename=="") return;

	//update settings
	QStringList refs = Settings::stringList("reference_files");
	refs.removeOne(name + "\t" + filename);
	Settings::setStringList("reference_files", refs);

	//update GUI
	loadReferenceFiles();
}
