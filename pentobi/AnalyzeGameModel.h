//-----------------------------------------------------------------------------
/** @file pentobi/AnalyzeGameModel.h
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#ifndef PENTOBI_ANALYZE_GAME_MODEL_H
#define PENTOBI_ANALYZE_GAME_MODEL_H

#include <QFutureWatcher>
#include <QQmlListProperty>
#include "libpentobi_mcts/AnalyzeGame.h"

class GameModel;
class PlayerModel;
namespace libpentobi_base { class Game; }
namespace libpentobi_mcts { class Search; }

using libpentobi_mcts::AnalyzeGame;
using libpentobi_mcts::Search;

//-----------------------------------------------------------------------------

class AnalyzeGameElement
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int moveColor MEMBER m_moveColor CONSTANT)
    Q_PROPERTY(double value MEMBER m_value CONSTANT)

public:
    explicit AnalyzeGameElement(QObject* parent, int moveColor, double value);

private:
    int m_moveColor;

    double m_value;
};

//-----------------------------------------------------------------------------

class AnalyzeGameModel
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<AnalyzeGameElement> elements READ elements NOTIFY elementsChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(int markMoveNumber READ markMoveNumber NOTIFY markMoveNumberChanged)

public:
    explicit AnalyzeGameModel(QObject* parent = nullptr);

    ~AnalyzeGameModel() override;


    Q_INVOKABLE void autoSave(GameModel* gameModel);

    Q_INVOKABLE void cancel();

    Q_INVOKABLE void clear();

    Q_INVOKABLE void gotoMove(GameModel* gameModel, int moveNumber);

    Q_INVOKABLE void loadAutoSave(GameModel* gameModel);

    Q_INVOKABLE void markCurrentMove(GameModel* gameModel);

    Q_INVOKABLE void start(GameModel* gameModel, PlayerModel* playerModel,
                           int nuSimulations);


    bool isRunning() const { return m_isRunning; }

    int markMoveNumber() const { return m_markMoveNumber; }

    QQmlListProperty<AnalyzeGameElement> elements();

signals:
    void isRunningChanged();

    void markMoveNumberChanged();

    void progressChanged();

    void elementsChanged();

private:
    bool m_isRunning = false;

    int m_markMoveNumber = -1;

    size_t m_nuSimulations;

    QList<AnalyzeGameElement*> m_elements;

    QFutureWatcher<void> m_watcher;

    AnalyzeGame m_analyzeGame;

    Search* m_search;


    Q_INVOKABLE void updateElements();


    void setIsRunning(bool isRunning);

    void setMarkMoveNumber(int markMoveNumber);
};

//-----------------------------------------------------------------------------

#endif // PENTOBI_ANALYZE_GAME_MODEL_H
