#include "State.h"
#include <sstream>
#include <vector>

State::State(const std::string& id, const std::string& name, StateType type)
    : id(id), name(name), type(type), position(0.0f, 0.0f)
{
}

State::~State()
{
}

std::string State::getId() const
{
    return id;
}

void State::setId(const std::string& id)
{
    this->id = id;
}

std::string State::getName() const
{
    return name;
}

void State::setName(const std::string& name)
{
    this->name = name;
}

StateType State::getType() const
{
    return type;
}

void State::setType(StateType type)
{
    this->type = type;
}

Point2D State::getPosition() const
{
    return position;
}

void State::setPosition(const Point2D& position)
{
    this->position = position;
}

bool State::isAcceptState() const
{
    return type == StateType::ACCEPT;
}

bool State::isRejectState() const
{
    return type == StateType::REJECT;
}

bool State::isStartState() const
{
    return type == StateType::START;
}

bool State::isNormalState() const
{
    return type == StateType::NORMAL;
}

std::string State::toString() const
{
    return id + "|" + name + "|" +
           std::to_string(static_cast<int>(type)) + "|" +
           std::to_string(position.x()) + "|" +
           std::to_string(position.y());
}

State State::fromString(const std::string& str)
{
    std::vector<std::string> parts;
    std::string part;
    std::stringstream ss(str);

    while (std::getline(ss, part, '|')) {
        parts.push_back(part);
    }

    if (parts.empty()) {
        return State("default");
    }

    std::string id = parts[0];
    std::string name = (parts.size() > 1) ? parts[1] : "";
    StateType type = (parts.size() > 2) ? static_cast<StateType>(std::stoi(parts[2])) : StateType::NORMAL;
    float posX = (parts.size() > 3) ? std::stof(parts[3]) : 0.0f;
    float posY = (parts.size() > 4) ? std::stof(parts[4]) : 0.0f;

    State state(id, name, type);
    state.setPosition(Point2D(posX, posY));
    return state;
}