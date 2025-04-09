#pragma once

#include <string>

// Replace QPointF with our own position class
class Point2D {
public:
    Point2D(float x = 0.0f, float y = 0.0f) : x_(x), y_(y) {}

    float x() const { return x_; }
    float y() const { return y_; }

private:
    float x_;
    float y_;
};

enum class StateType {
    NORMAL,
    START,
    ACCEPT,
    REJECT
};

class State {
public:
    // Constructor & destructor
    State(const std::string& id, const std::string& name = "", StateType type = StateType::NORMAL);
    ~State();

    // Core properties
    std::string getId() const;
    void setId(const std::string& id);

    std::string getName() const;
    void setName(const std::string& name);

    StateType getType() const;
    void setType(StateType type);

    Point2D getPosition() const;
    void setPosition(const Point2D& position);

    // Type checking methods
    bool isAcceptState() const;
    bool isRejectState() const;
    bool isStartState() const;
    bool isNormalState() const;

    // Serialization
    std::string toString() const;
    static State fromString(const std::string& str);

private:
    std::string id;
    std::string name;
    StateType type;
    Point2D position;
};