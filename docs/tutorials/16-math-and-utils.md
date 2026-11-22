# Math and utils

## Vectors

`Vector2`, `Vector3` and `Vector4` are templates with the aliases `Vector2f`, `Vector3f`, `Vector4f` (float) and
`Vector2i`, `Vector3i`, `Vector4i` (int). `Vector4f` is also used for RGBA colors.

```cpp
#include "SpinoCore/Math/Vector.h"

using SpinoCore::Math::Vector3f;

const Vector3f a{1.0f, 2.0f, 3.0f};
const Vector3f b = Vector3f::UnitY() * 2.0f;
const float dot = a.Dot(b);
const Vector3f normal = a.Cross(b).Normalized();
const Vector3f halfway = Vector3f::Lerp(a, b, 0.5f);
const bool same = Vector3f::NearlyEqual(a, Vector3f{1.0f, 2.0f, 3.00001f});
```

Arithmetic is component-wise. `Normalized` returns zero for a zero vector instead of NaN. `Project`,
`Rejection` and `Reflect` split and mirror vectors.

## Quaternions

```cpp
#include "SpinoCore/Math/Quaternion.h"

using SpinoCore::Math::Quaternionf;

const Quaternionf turn{Vector3f::UnitY(), SpinoCore::Math::Operations::ToRadians(90.0f)};
const Vector3f rotated = turn * Vector3f::NegUnitZ();
const Quaternionf facing = Quaternionf::LookRotation(target - eye);
const Quaternionf blend = Quaternionf::Slerp(Quaternionf::Identity(), turn, 0.25f);
```

`a * b` applies `b` first, then `a`. `LookRotation` points the local -Z axis along a direction.

## Matrices

`Matrix3f` and `Matrix4f` are column-major, as OpenGL expects; `m(row, col)` reads an element.
`MatrixFactory` builds transforms:

```cpp
#include "SpinoCore/Math/MatrixFactory.h"

namespace MF = SpinoCore::Math::MatrixFactory;

const auto world = MF::CreateTransform(Vector3f{0.0f, 1.0f, 0.0f}, turn, Vector3f::One());
const Vector3f point = world.TransformPoint(Vector3f::Zero());
const auto view = MF::CreateLookAt(eye, target, Vector3f::UnitY());
const auto projection = MF::CreatePerspectiveFOV(1.0f, 1280.0f, 720.0f, 0.1f, 100.0f);
```

## Scalars

`Math::Constants` has `PI`, `TwoPI`, `DegToRad`, `RadToDeg`, `Infinity` and `Tolerance`.
`Math::Operations` has `Abs`, `Sgn`, `Sqrt`, `Sin`, `Cos`, `Tan`, `Asin` and `Acos` (inputs clamped to
[-1, 1]), `Atan2`, `ToRadians`, `ToDegrees`, `NearlyEqual` and `NearlyZero`.

```cpp
const float angle = SpinoCore::Math::Constants::PI<float> / 4.0f;
```

## Random numbers

```cpp
#include "SpinoCore/Utils/Random.h"

namespace Random = SpinoCore::Utils::Random;

const float x = Random::GetFloatRange(-1.0f, 1.0f);
const int roll = Random::GetIntRange(1, 6);
Random::Seed(42);
```

Each thread has its own generator, seeded randomly. `Seed` makes the sequence repeatable for tests or replays.

## Sizes

`Utils::Size2D` is a width and height in pixels, used by the window and the UI.
