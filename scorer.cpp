#include "scorer.h"
#include "ui_scorer.h"
#include <QDebug>
#include <QDataStream>
#include <QIODevice>
#include <QFile>
#include <QTextStream>
#include <QtCharts>
#include <QChartView>
#include <QVector>
#include <QPainter>
#include <QString>
#include <QDate>
#include <algorithm>
#include <QColor>
#include <QGraphicsView>
#include <QPalette>
#include <QStyleFactory>
#include <QMessageBox>


using namespace QtCharts;

Scorer::Scorer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Scorer)
    , candidates()
{
    ui->setupUi(this);
    ui->graphicsView->setVisible(false);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

//	option to use palette
//    QPalette palette = QStyleFactory::create("Material")->standardPalette();
//    ui->graphicsView->setPalette(palette);

    index = 0;
    this->load();
    this->ui->centralwidget->setLayout(ui->horizontalLayout_main);
    this->setCentralWidget(ui->centralwidget);
    this->resize(400, 600);
    this->update_ui();
}

Scorer::~Scorer()
{
    delete ui;
}

auto Scorer::update_ui() -> void{

    assert(index>=0);
    assert(index<=candidates.size());

    // disable previous if first page
    (index == 0)?
        ui->pushButton_previous->setEnabled(false):
        ui->pushButton_previous->setEnabled(true);

    // write index and size of the list
    ui->label_index->setText(QString::number(index + 1));
    ui->label_total->setText(QString::number(candidates.size()));

    // if the index is validly inside the arrey, I can edit and delete
    if (index < candidates.size()){

        ui->pushButton_next->setEnabled(true);
        ui->pushButton_ConfirmEdit->setEnabled(true);

        // stampa linea on screen

        ui->pushButton_Delete->setEnabled(true);
        ui->lineEdit_nome->setText(candidates[index].nome);
        ui->lineEdit_cognome->setText(candidates[index].cognome);

        (candidates[index].genere == MALE)?
            ui->comboBox->setCurrentIndex(0):
            ui->comboBox->setCurrentIndex(1);

        ui->lineEdit_occupazione->setText(candidates[index].occupazione);
        ui->dateEdit->setDate(candidates[index].data_nascita);
        ui->lineEdit_score->setText(QString::number(candidates[index].score));

    }else{
        // siamo sul nuovo elemento
        // clearing input line
        clear();
        ui->pushButton_next->setEnabled(false);
        ui->pushButton_ConfirmEdit->setEnabled(false);
        ui->pushButton_Delete->setEnabled(false);
    }
    qDebug() << "updated UI";    // show entry index
}

// Function to read Candidates from a CSV file
QVector<Candidate> Scorer::importCSV(const QString& filename) {
    QVector<Candidate> candidates_csv;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            if (fields.size() == 5) {
                qDebug()<< fields[0];
                QString nome = fields[0].trimmed();
                QString cognome = fields[1].trimmed();
                gender genere = static_cast<gender>(fields[2].toInt());
                QString occupazione = fields[3].trimmed();
                QDate data_nascita = QDate::fromString(fields[4],"dd/MM/yyyy");
                int score = 0;
                Candidate candidate(nome,
                                    cognome,
                                    genere,
                                    occupazione,
                                    data_nascita,
                                    score);
                candidates_csv.append(candidate);
            }else{
                qDebug() << "bad ROW";
            }
        }
    }else{
        qDebug() << "file not loaded at all";
        QMessageBox::critical(nullptr, "Error, file not found",
                              "Please make sure it's called"
                              "data.csv and inside the directory");
        }
    file.close();
    return candidates_csv;
}

auto Scorer::Plot()->void{

    ui->graphicsView->setVisible(true);

    int strategy = plot_strategy;
    switch (strategy) {
    case 0:
            plot_age();
            break;
    case 1:
            plot_gender();
            break;
    case 2:
            plot_occupazione();
            break;
    default:
            assert(true);
            break;
    }
    this->plot_strategy = (++strategy)%3;
}

auto Scorer::plot_gender() -> void{
    int males = 0;
    int females = 0;
    for (int i = 0 ; i < candidates.size(); ++i){
            (candidates[i].genere == MALE)?
                ++males:
                ++females;
    }
    // Create the chart
    QtCharts::QChart *chart = new QtCharts::QChart();

    // Create the series
    QtCharts::QPieSeries *series = new QtCharts::QPieSeries();
    series->append("genere M", males);
    series->append("genere F", females);

    // Set custom colors for the slices
    QColor color1 = QColor(52, 152, 219);
    QColor color2 = QColor(155, 89, 182);

    series->slices().at(0)->setBrush(color1);
    series->slices().at(1)->setBrush(color2);


    // Set the hole size to make it a donut chart
    series->setHoleSize(0.55);

    // Add the series to the chart
    chart->addSeries(series);
    chart->setTitle("Candidati per genere");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    ui->graphicsView->setChart(chart);
    ui->graphicsView->fitInView(chart);
}



auto Scorer::plot_occupazione()->void{

    QMap<QString, QPair<int,int>> y_map;
    for (int i = 0; i < candidates.size(); i++) {
            QString occupazione = candidates[i].occupazione;
            if(y_map.contains(occupazione)){
                (candidates[i].genere == MALE)?
                y_map[occupazione].first += 1:
                y_map[occupazione].second +=1;
            }
            else{
                (candidates[i].genere==MALE)?
                y_map.insert(occupazione,QPair(1,0)):
                y_map.insert(occupazione,QPair(0,1));
            }
    }

    // Create the bar series
    QStackedBarSeries *series = new QStackedBarSeries();

    // Iterate over the QVector<QString, int> data and add bars to the series
    QStringList categories;
    QVector<int> males;
    QVector<int> females;

    // Populate the categories and values vectors
    // Assuming 'box_data_vector' contains the data
    for (auto it = y_map.begin(); it!= y_map.end(); it++ ) {
            categories.append(it.key());
            males.append(it.value().first);
            females.append(it.value().second);
    }

    // Create a bar set and add values to it
    QBarSet *barSetMales = new QBarSet("M");
    QBarSet *barSetFemales = new QBarSet("F");

    for (int i = 0; i < categories.size(); ++i) {
            *barSetMales << males[i];
            *barSetFemales << females[i];
    }

    barSetMales->setColor(QColor(52, 152, 219));
    barSetFemales->setColor(QColor(155, 89, 182));

    // Add the bar set to the series
    series->append(barSetMales);
    series->append(barSetFemales);


    // Create the chart and set the series
    QChart *chart = new QChart();
    chart->addSeries(series);

    chart->setTitle("Candidati per occupazione e genere");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create the X axis and set the categories
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Create the Y axis
    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Create the chart view and set the chart
//    QChartView *chartView = new QChartView(chart);
//    chartView->setRenderHint(QPainter::Antialiasing);
//    chartView->resize(500,500);
//    chartView->show();

    ui->graphicsView->setChart(chart);
    ui->graphicsView->fitInView(chart);

}


auto Scorer::plot_age()->void{

    QMap<int, int> y_map;
    for (int i = 0; i < candidates.size(); i++) {
            int birthYear = candidates[i].data_nascita.year();
            if(y_map.contains(birthYear)){
                y_map[birthYear]++;}
            else{
                y_map.insert(birthYear,1);
            }
    }

    QVector<QPair<int, int>> box_data_vector;

    for (auto it = y_map.begin(); it != y_map.end(); ++it) {
            QPair pair(it.key(), it.value());
            box_data_vector.append(pair);
    }

    std::sort(box_data_vector.begin(),
              box_data_vector.end(),
              [](const auto& a, const auto& b){ return a.first < b.first; });


    // Create the bar series
    QBarSeries *series = new QBarSeries();

    // Iterate over the QVector<QString, int> data and add bars to the series
    QStringList categories;
    QVector<int> values;

    // Populate the categories and values vectors
    // Assuming 'box_data_vector' contains the data
    for (const auto &pair : box_data_vector) {
            categories.append(QString::number(pair.first));
            values.append(pair.second);
    }

    // Create a bar set and add values to it
    QBarSet *barSet = new QBarSet("Numero di candidati");

    for (int i = 0; i < categories.size(); ++i) {
            *barSet << values[i];
    }

    // Add the bar set to the series
    series->append(barSet);

    // Create the chart and set the series
    QChart *chart = new QChart();
    chart->addSeries(series);

    chart->setTitle("Candidati per anno di nascita");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create the X axis and set the categories
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Create the Y axis
    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    ui->graphicsView->setChart(chart);
    ui->graphicsView->fitInView(chart);
}

//bool sortByFirstElement(const QPair<int, int> &pair1,
//                        const QPair<int, int> &pair2) {
//    return pair1.first < pair2.first;
//}


void Scorer::on_pushButton_ConfirmEdit_clicked()
{
    qDebug() << "pushed confirm";
    Candidate tmp = create_candidate_from_ui();
    if (check(tmp)){
        qDebug() << " cand ok";
        candidates[index]=tmp;
        save();
    }else{
        QMessageBox::warning(nullptr, "Warning", "Fill all the fields.");
        qDebug() << "cand not ok";
    }
    this->update_ui();
}


void Scorer::on_pushButton_addNew_clicked()
{
    qDebug() <<" pushed addNew";
    Candidate tmp = create_candidate_from_ui();
    if (this->check(tmp)){
        qDebug() << " cand ok";
        candidates.append(tmp);
        save();
    }else{
        QMessageBox::warning(nullptr, "Warning", "Fill all the fields.");
        qDebug() << "cand not ok";
    }
   this->update_ui();
}


void Scorer::on_pushButton_Plot_clicked()
{
    qDebug() <<" pushed PLOT";
    Plot();
    this->update_ui();
}


void Scorer::on_pushButton_previous_clicked()
{
    qDebug() <<" pushed prev";
    assert(index>0);
    --index;
    this->update_ui();
}


void Scorer::on_pushButton_next_clicked()
{
    qDebug() <<" pushed next";
    assert(index < candidates.size() );
    ++index;
    this->update_ui();
}


void Scorer::on_pushButton_Delete_clicked()
{
    qDebug() <<" pushed Delete";
    assert(index>=0);
    assert(index<candidates.size());
    candidates.remove(index);
    index = 0;
    save();
    this->update_ui();
}

void Scorer::on_pushButton_Read_CSV_clicked()
{
    qDebug() <<" pushed R_CSV";
    this->candidates = candidates + importCSV("data.csv");
    //ui->pushButton_Read_CSV->setEnabled(false);
    save();
    this->update_ui();
}

Candidate Scorer::create_candidate_from_ui(){
    Candidate tmp;
    QString tmp_nome;
    QString tmp_cognome;
    gender tmp_genere;
    QString tmp_occupazione;
    QDate tmp_data_nascita;
    int tmp_score;
    (ui->comboBox->currentText() == "M")?
       tmp_genere = MALE: tmp_genere = FEMALE;
    tmp_nome = ui->lineEdit_nome->text();
    tmp_cognome = ui->lineEdit_cognome->text();
    tmp_occupazione = ui->lineEdit_occupazione->text();
    bool success;
    if (ui->lineEdit_score->text().isEmpty()){
        tmp_score = 0;
    }else{
        int converted_score = ui->lineEdit_score
                              ->text().toInt(&success);
        if (success){
        tmp_score = converted_score;
        }else{
            QMessageBox::warning(nullptr, "Warning",
                             "The score must be Integer.");
            qDebug() << "NOT AN integer";
            tmp_score = 0;
        }
    }
    tmp_data_nascita = ui->dateEdit->date();
    return Candidate(
        tmp_nome,
        tmp_cognome,
        tmp_genere,
        tmp_occupazione,
        tmp_data_nascita,
        tmp_score);
}

auto Scorer::check(Candidate c)->bool{
    if( c.nome.isEmpty()||
        c.cognome.isEmpty()||
        c.occupazione.isEmpty())
        return false;
    return true;
}

auto Scorer::clear()-> void{
        ui->lineEdit_nome->setText("");
        ui->lineEdit_cognome->setText("");
        ui->comboBox->setCurrentIndex(0);
        ui->lineEdit_occupazione->setText("");
        ui->dateEdit->setDate(QDate::currentDate());
        ui->lineEdit_score->setText("");
}

auto Scorer::save() -> void{
        QFile file("data.serialized");
        if (file.open(QIODevice::ReadWrite)) {
        QDataStream stream(&file);
        stream << candidates;
        file.close();
        return;
        }
}

auto Scorer::load() -> void{
        QFile file("data.serialized");
        if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        stream >> candidates;
        file.close();
        return;
        }
}

// for serialization
QDataStream& operator<<(QDataStream& out, const Candidate& c) {
        out << c.nome;
        out << c.cognome;
        out << static_cast<int>(c.genere);
        out << c.occupazione;
        out << c.data_nascita;
        out << c.score;
        return out;
        }

QDataStream& operator>>(QDataStream& in, Candidate& c) {
        int genderValue;
        in >> c.nome >> c.cognome >> genderValue >> c.occupazione >> c.data_nascita >> c.score;
        c.genere = static_cast<gender>(genderValue);
        return in;
        }

