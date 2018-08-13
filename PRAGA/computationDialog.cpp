#include "computationDialog.h"
#include "climate.h"
#include "utilities.h"

extern Project myProject;


ComputationDialog::ComputationDialog(QSettings *settings, bool isAnomaly, bool isMeteoGrid, MainWindow *w)
    : settings(settings), isAnomaly(isAnomaly), isMeteoGrid(isMeteoGrid), QDialog(w)
{

    if (!isAnomaly)
    {
        setWindowTitle("Elaboration");
    }
    else
    {
        setWindowTitle("Reference Period");
    }

    QVBoxLayout mainLayout;
    QHBoxLayout varLayout;
    QHBoxLayout dateLayout;
    QHBoxLayout periodLayout;
    QHBoxLayout displayLayout;
    QHBoxLayout genericPeriodLayout;
    QHBoxLayout layoutOk;

    QHBoxLayout elaborationLayout;
    QHBoxLayout secondElabLayout;

//    setIsAnomaly(isAnomaly);
//    setIsMeteoGrid(isMeteoGrid);
//    setW(w);
    //setWindowTitle(title);


    meteoVariable var;

    if (!isAnomaly)
    {
        Q_FOREACH (QString group, settings->childGroups())
        {
            if (!group.endsWith("_VarToElab1"))
                continue;
            std::string item;
            std::string variable = group.left(group.size()-11).toStdString(); // remove "_VarToElab1"
            try {
              var = MapDailyMeteoVar.at(variable);
              item = MapDailyMeteoVarToString.at(var);
            }
            catch (const std::out_of_range& oor) {
              //TODO return false;
            }
            variableList.addItem(QString::fromStdString(item));
        }
    }
    else
    {
        var = myProject.clima->variable();
        std::string item = MapDailyMeteoVarToString.at(var);
        variableList.addItem(QString::fromStdString(item));
    }

    QLabel variableLabel("Variable: ");
    varLayout.addWidget(&variableLabel);
    varLayout.addWidget(&variableList);

    QCheckBox readReference("Read reference climate from db");
    varLayout.addWidget(&readReference);
    if (!isAnomaly)
    {
        readReference.setVisible(false);
    }

    currentDayLabel.setText("Day/Month:");
    currentDay.setDate(myProject.getCurrentDate());
    currentDay.setDisplayFormat("dd/MM");
    currentDayLabel.setBuddy(&currentDay);
    currentDayLabel.setVisible(true);
    currentDay.setVisible(true);

    int currentYear = myProject.getCurrentDate().year();
    QLabel firstDateLabel("Start Year:");
    //firstYearEdit.setPlaceholderText("yyyy");
    firstYearEdit.setText(QString::number(currentYear));
    firstYearEdit.setFixedWidth(110);
    firstYearEdit.setValidator(new QIntValidator(1800, 3000));
    firstDateLabel.setBuddy(&firstYearEdit);

    QLabel lastDateLabel("End Year:");
    //lastYearEdit.setPlaceholderText("yyyy");
    lastYearEdit.setText(QString::number(currentYear));
    lastYearEdit.setFixedWidth(110);
    lastYearEdit.setValidator(new QIntValidator(1800, 3000));
    lastDateLabel.setBuddy(&lastYearEdit);

    dateLayout.addWidget(&currentDayLabel);
    dateLayout.addWidget(&currentDay);

    dateLayout.addWidget(&firstDateLabel);
    dateLayout.addWidget(&firstYearEdit);

    dateLayout.addWidget(&lastDateLabel);
    dateLayout.addWidget(&lastYearEdit);

    periodTypeList.addItem("Daily");
    periodTypeList.addItem("Decadal");
    periodTypeList.addItem("Monthly");
    periodTypeList.addItem("Seasonal");
    periodTypeList.addItem("Annual");
    periodTypeList.addItem("Generic");

    QLabel periodTypeLabel("Period Type: ");
    periodLayout.addWidget(&periodTypeLabel);
    periodLayout.addWidget(&periodTypeList);

    QString periodSelected = periodTypeList.currentText();
    int dayOfYear = currentDay.date().dayOfYear();
    periodDisplay.setText("Day Of Year: " + QString::number(dayOfYear));
    periodDisplay.setReadOnly(true);

    displayLayout.addWidget(&periodDisplay);

    genericStartLabel.setText("Start Date:");
    genericPeriodStart.setDisplayFormat("dd/MM");
    genericStartLabel.setBuddy(&genericPeriodStart);
    genericEndLabel.setText("End Date:");
    genericPeriodEnd.setDisplayFormat("dd/MM");
    genericEndLabel.setBuddy(&genericPeriodEnd);
    nrYearLabel.setText("Delta Years:");
    nrYear.setValidator(new QIntValidator(-500, 500));
    nrYear.setText("0");
    nrYearLabel.setBuddy(&nrYear);

    genericStartLabel.setVisible(false);
    genericEndLabel.setVisible(false);
    genericPeriodStart.setVisible(false);
    genericPeriodEnd.setVisible(false);
    nrYearLabel.setVisible(false);
    nrYear.setVisible(false);

    genericPeriodLayout.addWidget(&genericStartLabel);
    genericPeriodLayout.addWidget(&genericPeriodStart);
    genericPeriodLayout.addWidget(&genericEndLabel);
    genericPeriodLayout.addWidget(&genericPeriodEnd);
    genericPeriodLayout.addWidget(&nrYearLabel);
    genericPeriodLayout.addWidget(&nrYear);

    elaborationLayout.addWidget(new QLabel("Elaboration: "));
    QString value = variableList.currentText();
    meteoVariable key = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, value.toStdString());
    std::string keyString = getKeyStringMeteoMap(MapDailyMeteoVar, key);
    QString group = QString::fromStdString(keyString)+"_VarToElab1";
    settings->beginGroup(group);
    int size = settings->beginReadArray(QString::fromStdString(keyString));
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString elab = settings->value("elab").toString();
        elaborationList.addItem( elab );
    }
    settings->endArray();
    settings->endGroup();
    elaborationLayout.addWidget(&elaborationList);

    elab1Parameter.setPlaceholderText("Parameter");
    elab1Parameter.setFixedWidth(90);
    elab1Parameter.setValidator(new QDoubleValidator(-9999.0, 9999.0, 2)); //LC accetta double con 2 cifre decimali da -9999 a 9999
    readParam.setText("Read param from db Climate");


    QString elab1Field = elaborationList.currentText();
    if ( MapElabWithParam.find(elab1Field.toStdString()) == MapElabWithParam.end())
    {
        elab1Parameter.clear();
        elab1Parameter.setReadOnly(true);
    }
    else
    {
        if (!readParam.isChecked())
        {
            elab1Parameter.setReadOnly(false);
        }
    }

    elaborationLayout.addWidget(&elab1Parameter);
    elaborationLayout.addWidget(&readParam);
    secondElabLayout.addWidget(new QLabel("Secondary Elaboration: "));

    group = elab1Field +"_Elab1Elab2";
    settings->beginGroup(group);
    secondElabList.addItem("None");
    size = settings->beginReadArray(elab1Field);
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString elab2 = settings->value("elab2").toString();
        secondElabList.addItem( elab2 );
    }
    settings->endArray();
    settings->endGroup();
    secondElabLayout.addWidget(&secondElabList);

    elab2Parameter.setPlaceholderText("Parameter");
    elab2Parameter.setFixedWidth(90);
    elab2Parameter.setValidator(new QDoubleValidator(-9999.0, 9999.0, 2)); //LC accetta double con 2 cifre decimali da -9999 a 9999

    QString elab2Field = secondElabList.currentText();
    if ( MapElabWithParam.find(elab2Field.toStdString()) == MapElabWithParam.end())
    {
        elab2Parameter.clear();
        elab2Parameter.setReadOnly(true);
    }
    else
    {
        elab2Parameter.setReadOnly(false);
    }

    secondElabLayout.addWidget(&elab2Parameter);

    connect(&firstYearEdit, &QLineEdit::editingFinished, [=](){ this->checkYears(); });
    connect(&lastYearEdit, &QLineEdit::editingFinished, [=](){ this->checkYears(); });

    connect(&variableList, &QComboBox::currentTextChanged, [=](const QString &newVar){ this->listElaboration(newVar); });
    connect(&currentDay, &QDateTimeEdit::dateChanged, [=](const QDate &newDate){ this->changeDate(newDate); });
    connect(&periodTypeList, &QComboBox::currentTextChanged, [=](const QString &newVar){ this->displayPeriod(newVar); });
    connect(&elaborationList, &QComboBox::currentTextChanged, [=](const QString &newElab){ this->listSecondElab(newElab); });
    connect(&secondElabList, &QComboBox::currentTextChanged, [=](const QString &newSecElab){ this->activeSecondParameter(newSecElab); });
    connect(&readParam, &QCheckBox::stateChanged, [=](int state){ this->readParameter(state); });

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    //connect(&buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(&buttonBox, &QDialogButtonBox::accepted, [=](){ this->done(true); });
    connect(&buttonBox, &QDialogButtonBox::rejected, [=](){ this->done(false); });

    layoutOk.addWidget(&buttonBox);

    mainLayout.addLayout(&varLayout);
    mainLayout.addLayout(&dateLayout);
    mainLayout.addLayout(&periodLayout);
    mainLayout.addLayout(&displayLayout);
    mainLayout.addLayout(&genericPeriodLayout);
    mainLayout.addLayout(&elaborationLayout);
    mainLayout.addLayout(&secondElabLayout);
    mainLayout.addLayout(&layoutOk);

    setLayout(&mainLayout);

    show();
    exec();

//    if (myProject.clima == NULL)
//    {
//        QMessageBox::information(NULL, "Error!", "clima is null...");
//        return false;
//    }

//    if (this->result() != QDialog::Accepted)
//    {
//        qInfo() << "return false";
//        return false;
//    }
//    else
//    {

//        periodSelected = periodTypeList.currentText();
//        value = variableList.currentText();
//        var = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, value.toStdString());


//        myProject.clima->setVariable(var);
//        myProject.clima->setYearStart(firstYearEdit.text().toInt());
//        myProject.clima->setYearEnd(lastYearEdit.text().toInt());
//        myProject.clima->setPeriodStr(periodSelected);
//        if (periodSelected == "Generic")
//        {
//            myProject.clima->setGenericPeriodDateStart(genericPeriodStart.date());
//            myProject.clima->setGenericPeriodDateEnd(genericPeriodEnd.date());
//            myProject.clima->setNYears(nrYear.text().toInt());
//        }
//        else
//        {
//            myProject.clima->setNYears(0);
//            QDate start;
//            QDate end;
//            getPeriodDates(periodSelected, firstYearEdit.text().toInt(), currentDay.date(), &start, &end);
//            myProject.clima->setNYears(start.year() - firstYearEdit.text().toInt());
//            myProject.clima->setGenericPeriodDateStart(start);
//            myProject.clima->setGenericPeriodDateEnd(end);
//        }

//        myProject.clima->setElab1(elaborationList.currentText());

//        if (!readParam.isChecked())
//        {
//            myProject.clima->setParam1IsClimate(false);
//            if (elab1Parameter.text() != "")
//            {
//                myProject.clima->setParam1(elab1Parameter.text().toFloat());
//            }
//            else
//            {
//                myProject.clima->setParam1(NODATA);
//            }
//        }
//        else
//        {
//            myProject.clima->setParam1IsClimate(true);
//            // TO DO LC

//        }
//        if (secondElabList.currentText() == "None" || secondElabList.currentText() == "No elaboration available")
//        {
//            myProject.clima->setElab2("");
//            myProject.clima->setParam2(NODATA);
//        }
//        else
//        {
//            myProject.clima->setElab2(secondElabList.currentText());
//            if (elab2Parameter.text() != "")
//            {
//                myProject.clima->setParam2(elab2Parameter.text().toFloat());
//            }
//            else
//            {
//                myProject.clima->setParam2(NODATA);
//            }
//        }

//        return true;
//    }
}


void ComputationDialog::done(bool res)
{
qInfo() << "computation: " << res << endl;
    if(res)  // ok was pressed
    {
        if (firstYearEdit.text().size() != 4)
        {
            QMessageBox::information(NULL, "Missing year", "Insert first year");
            return;
        }
        else if (lastYearEdit.text().size() != 4)
        {
            QMessageBox::information(NULL, "Missing year", "Insert last year");
            return;
        }
        else  // validate the data
        {
            if (firstYearEdit.text().toInt() > lastYearEdit.text().toInt())
            {
                QMessageBox::information(NULL, "Invalid year", "first year greater than last year");
                return;
            }
            if (elaborationList.currentText().toStdString() == "huglin" || elaborationList.currentText().toStdString() == "winkler" || elaborationList.currentText().toStdString() == "fregoni")
            {
                if (secondElabList.currentText().toStdString() == "None")
                {
                    QMessageBox::information(NULL, "Second Elaboration missing", elaborationList.currentText() + " requires second elaboration");
                    return;
                }

            }
            if ( MapElabWithParam.find(elaborationList.currentText().toStdString()) != MapElabWithParam.end())
            {
                if (elab1Parameter.text().isEmpty())
                {
                    QMessageBox::information(NULL, "Missing Parameter", "insert parameter");
                    return;
                }
            }
            if ( MapElabWithParam.find(secondElabList.currentText().toStdString()) != MapElabWithParam.end())
            {
                if (elab2Parameter.text().isEmpty())
                {
                    QMessageBox::information(NULL, "Missing Parameter", "insert second elaboration parameter");
                    return;
                }
            }
            if (periodTypeList.currentText() == "Generic")
            {
                if (nrYear.text().isEmpty())
                {
                    QMessageBox::information(NULL, "Missing Parameter", "insert Nr Years");
                    return;
                }
            }
            //QDialog::done(r);

            if (myProject.clima == NULL)
            {
                QMessageBox::information(NULL, "Error!", "clima is null...");
                return;
            }
qInfo() << "save in clima: " << endl;

            QString periodSelected = periodTypeList.currentText();
            QString value = variableList.currentText();
            meteoVariable var = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, value.toStdString());


            myProject.clima->setVariable(var);
            myProject.clima->setYearStart(firstYearEdit.text().toInt());
            myProject.clima->setYearEnd(lastYearEdit.text().toInt());
            myProject.clima->setPeriodStr(periodSelected);
            if (periodSelected == "Generic")
            {
                myProject.clima->setGenericPeriodDateStart(genericPeriodStart.date());
                myProject.clima->setGenericPeriodDateEnd(genericPeriodEnd.date());
                myProject.clima->setNYears(nrYear.text().toInt());
            }
            else
            {
                myProject.clima->setNYears(0);
                QDate start;
                QDate end;
                getPeriodDates(periodSelected, firstYearEdit.text().toInt(), currentDay.date(), &start, &end);
                myProject.clima->setNYears(start.year() - firstYearEdit.text().toInt());
                myProject.clima->setGenericPeriodDateStart(start);
                myProject.clima->setGenericPeriodDateEnd(end);
            }

            myProject.clima->setElab1(elaborationList.currentText());

            if (!readParam.isChecked())
            {
                myProject.clima->setParam1IsClimate(false);
                if (elab1Parameter.text() != "")
                {
                    myProject.clima->setParam1(elab1Parameter.text().toFloat());
                }
                else
                {
                    myProject.clima->setParam1(NODATA);
                }
            }
            else
            {
                myProject.clima->setParam1IsClimate(true);
                // TO DO LC

            }
            if (secondElabList.currentText() == "None" || secondElabList.currentText() == "No elaboration available")
            {
                myProject.clima->setElab2("");
                myProject.clima->setParam2(NODATA);
            }
            else
            {
                myProject.clima->setElab2(secondElabList.currentText());
                if (elab2Parameter.text() != "")
                {
                    myProject.clima->setParam2(elab2Parameter.text().toFloat());
                }
                else
                {
                    myProject.clima->setParam2(NODATA);
                }
            }


qInfo() << "elaboration: " << endl;
            if (!myProject.elaboration(isMeteoGrid, isAnomaly))
            {
                qInfo() << "elaboration error " << endl;
                myProject.logError();
            }
            else
            {
                qInfo() << "showElabResult start " << endl;
                w->showElabResult(true, isMeteoGrid, isAnomaly);
                qInfo() << "showElabResult end " << endl;
            }
            return;
        }

    }
    else    // cancel, close or exc was pressed
    {
        QDialog::done(QDialog::Rejected);
        return;
    }

}

/*
bool ComputationDialog::computation(bool isAnomaly)
{
    QVBoxLayout mainLayout;
    QHBoxLayout varLayout;
    QHBoxLayout dateLayout;
    QHBoxLayout periodLayout;
    QHBoxLayout displayLayout;
    QHBoxLayout genericPeriodLayout;
    QHBoxLayout layoutOk;

    QHBoxLayout elaborationLayout;
    QHBoxLayout secondElabLayout;

    setWindowTitle(title);
    QComboBox variableList;
    meteoVariable var;

    if (!isAnomaly)
    {
        Q_FOREACH (QString group, settings->childGroups())
        {
            if (!group.endsWith("_VarToElab1"))
                continue;
            std::string item;
            std::string variable = group.left(group.size()-11).toStdString(); // remove "_VarToElab1"
            try {
              var = MapDailyMeteoVar.at(variable);
              item = MapDailyMeteoVarToString.at(var);
            }
            catch (const std::out_of_range& oor) {
              return false;
            }
            variableList.addItem(QString::fromStdString(item));
        }
    }
    else
    {
        var = myProject.clima->variable();
        std::string item = MapDailyMeteoVarToString.at(var);
        variableList.addItem(QString::fromStdString(item));
    }

    QLabel variableLabel("Variable: ");
    varLayout.addWidget(&variableLabel);
    varLayout.addWidget(&variableList);

    QCheckBox readReference("Read reference climate from db");
    varLayout.addWidget(&readReference);
    if (!isAnomaly)
    {
        readReference.setVisible(false);
    }

    currentDayLabel.setText("Day/Month:");
    currentDay.setDate(myProject.getCurrentDate());
    currentDay.setDisplayFormat("dd/MM");
    currentDayLabel.setBuddy(&currentDay);
    currentDayLabel.setVisible(true);
    currentDay.setVisible(true);

    int currentYear = myProject.getCurrentDate().year();
    QLabel firstDateLabel("Start Year:");
    //firstYearEdit.setPlaceholderText("yyyy");
    firstYearEdit.setText(QString::number(currentYear));
    firstYearEdit.setFixedWidth(110);
    firstYearEdit.setValidator(new QIntValidator(1800, 3000));
    firstDateLabel.setBuddy(&firstYearEdit);

    QLabel lastDateLabel("End Year:");
    //lastYearEdit.setPlaceholderText("yyyy");
    lastYearEdit.setText(QString::number(currentYear));
    lastYearEdit.setFixedWidth(110);
    lastYearEdit.setValidator(new QIntValidator(1800, 3000));
    lastDateLabel.setBuddy(&lastYearEdit);

    dateLayout.addWidget(&currentDayLabel);
    dateLayout.addWidget(&currentDay);

    dateLayout.addWidget(&firstDateLabel);
    dateLayout.addWidget(&firstYearEdit);

    dateLayout.addWidget(&lastDateLabel);
    dateLayout.addWidget(&lastYearEdit);

    periodTypeList.addItem("Daily");
    periodTypeList.addItem("Decadal");
    periodTypeList.addItem("Monthly");
    periodTypeList.addItem("Seasonal");
    periodTypeList.addItem("Annual");
    periodTypeList.addItem("Generic");

    QLabel periodTypeLabel("Period Type: ");
    periodLayout.addWidget(&periodTypeLabel);
    periodLayout.addWidget(&periodTypeList);

    QString periodSelected = periodTypeList.currentText();
    int dayOfYear = currentDay.date().dayOfYear();
    periodDisplay.setText("Day Of Year: " + QString::number(dayOfYear));
    periodDisplay.setReadOnly(true);

    displayLayout.addWidget(&periodDisplay);

    genericStartLabel.setText("Start Date:");
    genericPeriodStart.setDisplayFormat("dd/MM");
    genericStartLabel.setBuddy(&genericPeriodStart);
    genericEndLabel.setText("End Date:");
    genericPeriodEnd.setDisplayFormat("dd/MM");
    genericEndLabel.setBuddy(&genericPeriodEnd);
    nrYearLabel.setText("Delta Years:");
    nrYear.setValidator(new QIntValidator(-500, 500));
    nrYear.setText("0");
    nrYearLabel.setBuddy(&nrYear);

    genericStartLabel.setVisible(false);
    genericEndLabel.setVisible(false);
    genericPeriodStart.setVisible(false);
    genericPeriodEnd.setVisible(false);
    nrYearLabel.setVisible(false);
    nrYear.setVisible(false);

    genericPeriodLayout.addWidget(&genericStartLabel);
    genericPeriodLayout.addWidget(&genericPeriodStart);
    genericPeriodLayout.addWidget(&genericEndLabel);
    genericPeriodLayout.addWidget(&genericPeriodEnd);
    genericPeriodLayout.addWidget(&nrYearLabel);
    genericPeriodLayout.addWidget(&nrYear);

    elaborationLayout.addWidget(new QLabel("Elaboration: "));
    QString value = variableList.currentText();
    meteoVariable key = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, value.toStdString());
    std::string keyString = getKeyStringMeteoMap(MapDailyMeteoVar, key);
    QString group = QString::fromStdString(keyString)+"_VarToElab1";
    settings->beginGroup(group);
    int size = settings->beginReadArray(QString::fromStdString(keyString));
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString elab = settings->value("elab").toString();
        elaborationList.addItem( elab );
    }
    settings->endArray();
    settings->endGroup();
    elaborationLayout.addWidget(&elaborationList);

    elab1Parameter.setPlaceholderText("Parameter");
    elab1Parameter.setFixedWidth(90);
    elab1Parameter.setValidator(new QDoubleValidator(-9999.0, 9999.0, 2)); //LC accetta double con 2 cifre decimali da -9999 a 9999
    QCheckBox readParam("Read param from db Climate");


    QString elab1Field = elaborationList.currentText();
    if ( MapElabWithParam.find(elab1Field.toStdString()) == MapElabWithParam.end())
    {
        elab1Parameter.clear();
        elab1Parameter.setReadOnly(true);
    }
    else
    {
        if (!readParam.isChecked())
        {
            elab1Parameter.setReadOnly(false);
        }
    }

    elaborationLayout.addWidget(&elab1Parameter);
    elaborationLayout.addWidget(&readParam);
    secondElabLayout.addWidget(new QLabel("Secondary Elaboration: "));

    group = elab1Field +"_Elab1Elab2";
    settings->beginGroup(group);
    secondElabList.addItem("None");
    size = settings->beginReadArray(elab1Field);
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString elab2 = settings->value("elab2").toString();
        secondElabList.addItem( elab2 );
    }
    settings->endArray();
    settings->endGroup();
    secondElabLayout.addWidget(&secondElabList);

    elab2Parameter.setPlaceholderText("Parameter");
    elab2Parameter.setFixedWidth(90);
    elab2Parameter.setValidator(new QDoubleValidator(-9999.0, 9999.0, 2)); //LC accetta double con 2 cifre decimali da -9999 a 9999

    QString elab2Field = secondElabList.currentText();
    if ( MapElabWithParam.find(elab2Field.toStdString()) == MapElabWithParam.end())
    {
        elab2Parameter.clear();
        elab2Parameter.setReadOnly(true);
    }
    else
    {
        elab2Parameter.setReadOnly(false);
    }

    secondElabLayout.addWidget(&elab2Parameter);

    connect(&firstYearEdit, &QLineEdit::editingFinished, [=](){ this->checkYears(); });
    connect(&lastYearEdit, &QLineEdit::editingFinished, [=](){ this->checkYears(); });

    connect(&variableList, &QComboBox::currentTextChanged, [=](const QString &newVar){ this->listElaboration(newVar); });
    connect(&currentDay, &QDateTimeEdit::dateChanged, [=](const QDate &newDate){ this->changeDate(newDate); });
    connect(&periodTypeList, &QComboBox::currentTextChanged, [=](const QString &newVar){ this->displayPeriod(newVar); });
    connect(&elaborationList, &QComboBox::currentTextChanged, [=](const QString &newElab){ this->listSecondElab(newElab); });
    connect(&secondElabList, &QComboBox::currentTextChanged, [=](const QString &newSecElab){ this->activeSecondParameter(newSecElab); });
    connect(&readParam, &QCheckBox::stateChanged, [=](int state){ this->readParameter(state); });

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layoutOk.addWidget(&buttonBox);

    mainLayout.addLayout(&varLayout);
    mainLayout.addLayout(&dateLayout);
    mainLayout.addLayout(&periodLayout);
    mainLayout.addLayout(&displayLayout);
    mainLayout.addLayout(&genericPeriodLayout);
    mainLayout.addLayout(&elaborationLayout);
    mainLayout.addLayout(&secondElabLayout);
    mainLayout.addLayout(&layoutOk);

    setLayout(&mainLayout);

    exec();

    if (myProject.clima == NULL)
    {
        QMessageBox::information(NULL, "Error!", "clima is null...");
        return false;
    }

    if (this->result() != QDialog::Accepted)
    {
        qInfo() << "return false";
        return false;
    }
    else
    {

        periodSelected = periodTypeList.currentText();
        value = variableList.currentText();
        var = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, value.toStdString());


        myProject.clima->setVariable(var);
        myProject.clima->setYearStart(firstYearEdit.text().toInt());
        myProject.clima->setYearEnd(lastYearEdit.text().toInt());
        myProject.clima->setPeriodStr(periodSelected);
        if (periodSelected == "Generic")
        {
            myProject.clima->setGenericPeriodDateStart(genericPeriodStart.date());
            myProject.clima->setGenericPeriodDateEnd(genericPeriodEnd.date());
            myProject.clima->setNYears(nrYear.text().toInt());
        }
        else
        {
            myProject.clima->setNYears(0);
            QDate start;
            QDate end;
            getPeriodDates(periodSelected, firstYearEdit.text().toInt(), currentDay.date(), &start, &end);
            myProject.clima->setNYears(start.year() - firstYearEdit.text().toInt());
            myProject.clima->setGenericPeriodDateStart(start);
            myProject.clima->setGenericPeriodDateEnd(end);
        }

        myProject.clima->setElab1(elaborationList.currentText());

        if (!readParam.isChecked())
        {
            myProject.clima->setParam1IsClimate(false);
            if (elab1Parameter.text() != "")
            {
                myProject.clima->setParam1(elab1Parameter.text().toFloat());
            }
            else
            {
                myProject.clima->setParam1(NODATA);
            }
        }
        else
        {
            myProject.clima->setParam1IsClimate(true);
            // TO DO LC

        }
        if (secondElabList.currentText() == "None" || secondElabList.currentText() == "No elaboration available")
        {
            myProject.clima->setElab2("");
            myProject.clima->setParam2(NODATA);
        }
        else
        {
            myProject.clima->setElab2(secondElabList.currentText());
            if (elab2Parameter.text() != "")
            {
                myProject.clima->setParam2(elab2Parameter.text().toFloat());
            }
            else
            {
                myProject.clima->setParam2(NODATA);
            }
        }

        return true;
    }
}


void ComputationDialog::done(int r)
{

    if(QDialog::Accepted == r)  // ok was pressed
    {
        if(firstYearEdit.text().size() == 4 && lastYearEdit.text().size() == 4)   // validate the data
        {
            if (firstYearEdit.text().toInt() > lastYearEdit.text().toInt())
            {
                QMessageBox::information(NULL, "Invalid year", "first year greater than last year");
                return;
            }
            if (elaborationList.currentText().toStdString() == "huglin" || elaborationList.currentText().toStdString() == "winkler" || elaborationList.currentText().toStdString() == "fregoni")
            {
                if (secondElabList.currentText().toStdString() == "None")
                {
                    QMessageBox::information(NULL, "Second Elaboration missing", elaborationList.currentText() + " requires second elaboration");
                    return;
                }

            }
            if ( MapElabWithParam.find(elaborationList.currentText().toStdString()) != MapElabWithParam.end())
            {
                if (elab1Parameter.text().isEmpty())
                {
                    QMessageBox::information(NULL, "Missing Parameter", "insert parameter");
                    return;
                }
            }
            if ( MapElabWithParam.find(secondElabList.currentText().toStdString()) != MapElabWithParam.end())
            {
                if (elab2Parameter.text().isEmpty())
                {
                    QMessageBox::information(NULL, "Missing Parameter", "insert second elaboration parameter");
                    return;
                }
            }
            if (periodTypeList.currentText() == "Generic")
            {
                if (nrYear.text().isEmpty())
                {
                    QMessageBox::information(NULL, "Missing Parameter", "insert Nr Years");
                    return;
                }
            }
            QDialog::done(r);
            return;
        }
        else if (firstYearEdit.text().size() != 4)
        {
            QMessageBox::information(NULL, "Missing year", "Insert first year");
            return;
        }
        else
        {
            QMessageBox::information(NULL, "Missing year", "Insert last year");
            return;
        }

    }
    else    // cancel, close or exc was pressed
    {
        QDialog::done(r);
        return;
    }

}
*/
void ComputationDialog::checkYears()
{
    if (firstYearEdit.text().toInt() == lastYearEdit.text().toInt())
    {
        secondElabList.clear();
        secondElabList.addItem("No elaboration available");
    }
    else
    {
        listSecondElab(elaborationList.currentText());
    }
}

void ComputationDialog::changeDate(const QDate newDate)
{
    displayPeriod(periodTypeList.currentText());
}


void ComputationDialog::displayPeriod(const QString value)
{

    if (value == "Daily")
    {
        periodDisplay.setVisible(true);
        currentDayLabel.setVisible(true);
        currentDay.setVisible(true);
        genericStartLabel.setVisible(false);
        genericEndLabel.setVisible(false);
        genericPeriodStart.setVisible(false);
        genericPeriodEnd.setVisible(false);
        nrYearLabel.setVisible(false);
        nrYear.setVisible(false);
        int dayOfYear = currentDay.date().dayOfYear();
        periodDisplay.setText("Day Of Year: " + QString::number(dayOfYear));
    }
    else if (value == "Decadal")
    {
        periodDisplay.setVisible(true);
        currentDayLabel.setVisible(true);
        currentDay.setVisible(true);
        genericStartLabel.setVisible(false);
        genericEndLabel.setVisible(false);
        genericPeriodStart.setVisible(false);
        genericPeriodEnd.setVisible(false);
        nrYearLabel.setVisible(false);
        nrYear.setVisible(false);
        int decade = decadeFromDate(currentDay.date());
        periodDisplay.setText("Decade: " + QString::number(decade));
    }
    else if (value == "Monthly")
    {
        periodDisplay.setVisible(true);
        currentDayLabel.setVisible(true);
        currentDay.setVisible(true);
        genericStartLabel.setVisible(false);
        genericEndLabel.setVisible(false);
        genericPeriodStart.setVisible(false);
        genericPeriodEnd.setVisible(false);
        nrYearLabel.setVisible(false);
        nrYear.setVisible(false);
        periodDisplay.setText("Month: " + QString::number(currentDay.date().month()));
    }
    else if (value == "Seasonal")
    {
        periodDisplay.setVisible(true);
        currentDayLabel.setVisible(true);
        currentDay.setVisible(true);
        genericStartLabel.setVisible(false);
        genericEndLabel.setVisible(false);
        genericPeriodStart.setVisible(false);
        genericPeriodEnd.setVisible(false);
        nrYearLabel.setVisible(false);
        nrYear.setVisible(false);
        QString season = getStringSeasonFromDate(currentDay.date());
        periodDisplay.setText("Season: " + season);
    }
    else if (value == "Annual")
    {
        periodDisplay.setVisible(false);
        currentDayLabel.setVisible(true);
        currentDay.setVisible(true);
        genericStartLabel.setVisible(false);
        genericEndLabel.setVisible(false);
        genericPeriodStart.setVisible(false);
        genericPeriodEnd.setVisible(false);
        nrYearLabel.setVisible(false);
        nrYear.setVisible(false);
    }
    else if (value == "Generic")
    {
        periodDisplay.setVisible(false);
        currentDayLabel.setVisible(false);
        currentDay.setVisible(false);

        genericStartLabel.setVisible(true);
        genericEndLabel.setVisible(true);
        genericPeriodStart.setVisible(true);
        genericPeriodEnd.setVisible(true);

        nrYearLabel.setVisible(true);
        nrYear.setVisible(true);
        nrYear.setText("0");
        nrYear.setEnabled(true);

        if (elaborationList.currentText().toStdString() == "huglin" || elaborationList.currentText().toStdString() == "fregoni")
        {
            QDate fixStart(firstYearEdit.text().toInt(),4,1);
            QDate fixEnd(lastYearEdit.text().toInt(),9,30);
            genericPeriodStart.setDate(fixStart);
            genericPeriodStart.setDisplayFormat("dd/MM");
            genericPeriodEnd.setDisplayFormat("dd/MM");
            genericPeriodEnd.setDate(fixEnd);
            nrYear.setText("0");
            nrYear.setEnabled(false);
        }
        else if (elaborationList.currentText().toStdString() == "winkler")
        {
            QDate fixStart(firstYearEdit.text().toInt(),4,1);
            QDate fixEnd(lastYearEdit.text().toInt(),10,31);
            genericPeriodStart.setDate(fixStart);
            genericPeriodStart.setDisplayFormat("dd/MM");
            genericPeriodEnd.setDisplayFormat("dd/MM");
            genericPeriodEnd.setDate(fixEnd);
            nrYear.setText("0");
            nrYear.setEnabled(false);
        }
        else
        {
            QDate defaultStart(firstYearEdit.text().toInt(),1,1);
            QDate defaultEnd(lastYearEdit.text().toInt(),1,1);
            genericPeriodStart.setDate(defaultStart);
            genericPeriodStart.setDisplayFormat("dd/MM");
            genericPeriodEnd.setDisplayFormat("dd/MM");
            genericPeriodEnd.setDate(defaultEnd);
            nrYear.setText("0");
            nrYear.setEnabled(true);
        }

    }

}

void ComputationDialog::listElaboration(const QString value)
{

    meteoVariable key = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, value.toStdString());
    std::string keyString = getKeyStringMeteoMap(MapDailyMeteoVar, key);
    QString group = QString::fromStdString(keyString)+"_VarToElab1";
    settings->beginGroup(group);
    int size = settings->beginReadArray(QString::fromStdString(keyString));
    elaborationList.clear();

    for (int i = 0; i < size; ++i)
    {
        settings->setArrayIndex(i);
        QString elab = settings->value("elab").toString();
        elaborationList.addItem( elab );

    }
    settings->endArray();
    settings->endGroup();

    listSecondElab(elaborationList.currentText());

}

void ComputationDialog::listSecondElab(const QString value)
{

    if ( MapElabWithParam.find(value.toStdString()) == MapElabWithParam.end())
    {
        elab1Parameter.clear();
        elab1Parameter.setReadOnly(true);
    }
    else
    {
        elab1Parameter.setReadOnly(false);
    }

    if (elaborationList.currentText().toStdString() == "huglin" || elaborationList.currentText().toStdString() == "winkler" || elaborationList.currentText().toStdString() == "fregoni")
    {
        periodTypeList.setCurrentText("Generic");
        periodTypeList.setEnabled(false);
    }
    else
    {
        periodTypeList.setEnabled(true);
        nrYear.setEnabled(true);
    }

    displayPeriod(periodTypeList.currentText());

    QString group = value + "_Elab1Elab2";
    settings->beginGroup(group);
    int size = settings->beginReadArray(value);

    if (size == 0 || firstYearEdit.text().toInt() == lastYearEdit.text().toInt())
    {
        secondElabList.clear();
        secondElabList.addItem("No elaboration available");
        settings->endArray();
        settings->endGroup();
        return;
    }
    secondElabList.clear();
    secondElabList.addItem("None");
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString elab2 = settings->value("elab2").toString();
        secondElabList.addItem( elab2 );
    }
    settings->endArray();
    settings->endGroup();

}

void ComputationDialog::activeSecondParameter(const QString value)
{

        if ( MapElabWithParam.find(value.toStdString()) == MapElabWithParam.end())
        {
            elab2Parameter.clear();
            elab2Parameter.setReadOnly(true);
        }
        else
        {
            elab2Parameter.setReadOnly(false);
        }
}

void ComputationDialog::readParameter(int state)
{
    if (state)
    {
        elab1Parameter.clear();
        elab1Parameter.setReadOnly(true);
    }
    else
    {
        elab1Parameter.setReadOnly(false);
    }
}


//QSettings *ComputationDialog::getSettings() const
//{
//    return settings;
//}

//void ComputationDialog::setSettings(QSettings *value)
//{
//    settings = value;
//}

//QString ComputationDialog::getTitle() const
//{
//    return title;
//}

//void ComputationDialog::setTitle(const QString &value)
//{
//    title = value;
//}

//bool ComputationDialog::getIsAnomaly() const
//{
//    return isAnomaly;
//}

//void ComputationDialog::setIsAnomaly(bool value)
//{
//    isAnomaly = value;
//}

//bool ComputationDialog::getIsMeteoGrid() const
//{
//    return isMeteoGrid;
//}

//void ComputationDialog::setIsMeteoGrid(bool value)
//{
//    isMeteoGrid = value;
//}

//MainWindow *ComputationDialog::getW() const
//{
//    return w;
//}

//void ComputationDialog::setW(MainWindow *value)
//{
//    w = value;
//}
