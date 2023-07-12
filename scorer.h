#ifndef SCORER_H
#define SCORER_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class Scorer; }
QT_END_NAMESPACE

enum gender{
    MALE,
    FEMALE
};

struct Candidate {
    QString nome;
    QString cognome;
    gender genere;
    QString occupazione;
    QDate data_nascita;
    int score;
    Candidate(){}
    Candidate(
        const QString &n,
        const QString &c,
        const gender &g,
        const QString &o,
        const QDate &dn,
        const int &s):
        nome(n),
        cognome(c),
        genere(g),
        occupazione(o),
        data_nascita(dn),
        score(s)
    {}
    Candidate& operator=(const Candidate& other) {
        if (this != &other) {
            nome = other.nome;
            cognome = other.cognome;
            genere = other.genere;
            occupazione = other.occupazione;
            data_nascita = other.data_nascita;
            score = other.score;
        }
        return *this;
    }
    Candidate(const Candidate& other):
        nome(other.nome),
        cognome(other.cognome),
        genere(other.genere),
        occupazione(other.occupazione),
        data_nascita(other.data_nascita),
        score(other.score)
    {}
};


class Scorer : public QMainWindow
{
    Q_OBJECT

public:
    Scorer(QWidget *parent = nullptr);
    ~Scorer();


private slots:
    void on_pushButton_ConfirmEdit_clicked();
    void on_pushButton_addNew_clicked();
    void on_pushButton_Plot_clicked();
    void on_pushButton_previous_clicked();
    void on_pushButton_next_clicked();
    void on_pushButton_Delete_clicked();
    void on_pushButton_Read_CSV_clicked();

private:
    Ui::Scorer *ui;
    QVector<Candidate> candidates;
    int index;
    int plot_strategy;
    auto importCSV(const QString& filename) -> QVector<Candidate>;
    void Plot();
    auto plot_age()-> void;
    auto plot_gender()-> void;
    auto plot_occupazione()->void;

    auto update_ui() -> void;
    auto create_candidate_from_ui() -> Candidate;
    auto check(Candidate c)->bool;
    auto clear() ->void;
    auto save()->void;
    auto load()->void;
    //auto sortByFirstElement(const QPair<int, int> &pair1,
    //                        const QPair<int, int> &pair2)
    //    ->bool;
};
#endif // SCORER_H
