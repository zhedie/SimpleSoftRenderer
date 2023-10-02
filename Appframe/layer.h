#pragma once

class Layer {
public:
    virtual ~Layer() = default;
    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate(float timeStep) {}
    virtual void onUIRender() {}
};