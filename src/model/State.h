#ifndef STATE_H
#define STATE_H

#include <string>
#include <QPointF>

enum class StateType {
    NORMAL,
    START,
    ACCEPT,
    REJECT
};

class State {
public:
    State(const std::string& id, const std::string& name = "", StateType type = StateType::NORMAL);
    ~State();
    
    // Getters and setters
    std::string getId() const;
    void setId(const std::string& id);
    
    std::string getName() const;
    void setName(const std::string& name);
    
    StateType getType() const;
    void setType(StateType type);
    
    QPointF getPosition() const;
    void setPosition(const QPointF& position);
    
    // Utility
    bool isAcceptState() const;
    bool isRejectState() const;
    bool isStartState() const;
    bool isNormalState() const;
    
    // For serialization
    std::string toString() const;
    static State fromString(const std::string& str);

private:
    std::string id;
    std::string name;
    StateType type;
    QPointF position;
};

#endif // STATE_H