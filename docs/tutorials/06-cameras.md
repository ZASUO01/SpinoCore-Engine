# Cameras

The 3D scene is drawn with the current camera of the `CamerasManager`. A default camera always exists, and
any camera can be added by name.

## Writing a camera

A camera sets the protected `mEye`, `mTarget`, `mUp`, `mFov` (radians), `mNear` and `mFar`, then calls
`Apply3DView()`. The engine calls `SetViewportSize` on resizes, which rebuilds the projection.

```cpp
#include "SpinoCore/Core/Cameras/Camera.h"
#include "SpinoCore/Math/Math.h"

class OrbitCamera final : public SpinoCore::Core::Cameras::Camera {
public:
    explicit OrbitCamera(const float radius) : mRadius(radius) {
        mFov = 60.0f * SpinoCore::Math::Constants::DegToRad<float>;
        mNear = 0.1f;
        mFar = 100.0f;
    }

    void Update(const float deltaTime) override {
        using namespace SpinoCore::Math::Operations;
        mAngle += 0.5f * deltaTime;
        mEye = {Cos(mAngle) * mRadius, 3.0f, Sin(mAngle) * mRadius};
        Apply3DView();
    }

private:
    float mRadius;
    float mAngle{0.0f};
};
```

`Update` runs once per simulation step for the current camera.

## Using it

```cpp
void OnSetup(const ControlContext& ctx) override {
    ctx.camerasManager.AddCamera(std::make_unique<OrbitCamera>(8.0f), "orbit");
    ctx.camerasManager.SetCurrentCamera("orbit");
}
```

| Call | Effect |
|---|---|
| `AddCamera` | Adds a camera; a name in use is ignored. |
| `GetCamera` | Returns the camera or nullptr. |
| `SetCurrentCamera` | Draws with that camera. |
| `RemoveCamera` | Removes it; removing the current one selects the default camera. |
| `CleanUp` | Removes every camera except the default one. |

The engine cleans up the cameras when the scene changes.
