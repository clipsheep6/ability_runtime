/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_FRAMEWORKS_COMPONENTS_NG_PROPERTIES_TRANSITION_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_COMPONENTS_NG_PROPERTIES_TRANSITION_PROPERTY_H

#include <iomanip>
#include <ios>
#include <optional>
#include <sstream>
#include <string>

#include "base/geometry/dimension.h"
#include "core/animation/animation_pub.h"
#include "core/components/common/properties/animation_option.h"
#include "core/components_ng/property/property.h"


namespace OHOS::Ace::NG {

struct TranslateOptions {
    Dimension x;
    Dimension y;
    Dimension z;
    TranslateOptions() = default;
    TranslateOptions(const Dimension& x, const Dimension& y, const Dimension& z) : x(x), y(y), z(z) {}
    // for inner construct, default unit is PX
    TranslateOptions(float x, float y, float z) : x(x), y(y), z(z) {}
    bool operator==(const TranslateOptions& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
    std::string ToString() const
    {
        return "translate:[" + x.ToString() + ", " + y.ToString() + ", " + z.ToString() + "]";
    }
};
struct ScaleOptions {
    float xScale = 1.0f;
    float yScale = 1.0f;
    float zScale = 1.0f;
    Dimension centerX;
    Dimension centerY;
    ScaleOptions(float xScale, float yScale, float zScale, const Dimension& centerX, const Dimension& centerY)
        : xScale(xScale), yScale(yScale), zScale(zScale), centerX(centerX), centerY(centerY)
    {}
    ScaleOptions() = default;
    bool operator==(const ScaleOptions& other) const
    {
        return NearEqual(xScale, other.xScale) && NearEqual(yScale, other.yScale) && NearEqual(zScale, other.zScale) &&
               NearEqual(centerX, other.centerX) && NearEqual(centerY, other.centerY);
    }
    std::string ToString() const
    {
        return "scale:[" + std::to_string(xScale) + "," + std::to_string(yScale) + "," + std::to_string(zScale) + "," +
               centerX.ToString() + "," + centerY.ToString() + "]";
    }
};
struct RotateOptions {
    float xDirection = 0.0f;
    float yDirection = 0.0f;
    float zDirection = 0.0f;
    // angle in degree unit
    float angle = 0.0f;
    Dimension centerX;
    Dimension centerY;
    Dimension centerZ;
    RotateOptions(float xDirection, float yDirection, float zDirection, float angle, const Dimension& centerX,
        const Dimension& centerY)
        : xDirection(xDirection), yDirection(yDirection), zDirection(zDirection), angle(angle), centerX(centerX),
          centerY(centerY)
    {}
    RotateOptions() = default;
    bool operator==(const RotateOptions& other) const
    {
        return NearEqual(angle, other.angle) && NearEqual(xDirection, other.xDirection) &&
               NearEqual(yDirection, other.yDirection) && NearEqual(zDirection, other.zDirection) &&
               NearEqual(centerX, other.centerX) && NearEqual(centerY, other.centerY) &&
               NearEqual(centerZ, other.centerZ);
    }
    std::string ToString() const
    {
        return "rotate:[" + std::to_string(xDirection) + "," + std::to_string(yDirection) + "," +
               std::to_string(zDirection) + "," + centerX.ToString() + "," + centerY.ToString() +
               ", cameraDistance: " + centerZ.ToString() + ", angle:" + std::to_string(angle) + "]";
    }
};
struct TransitionOptions {
    TransitionType Type = TransitionType::ALL;
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Opacity, float);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Translate, TranslateOptions);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Scale, ScaleOptions);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(Rotate, RotateOptions);
    static TransitionOptions GetDefaultTransition(TransitionType type)
    {
        TransitionOptions options;
        options.Type = type;
        options.UpdateOpacity(0.0f);
        return options;
    }
    bool operator==(const TransitionOptions& other) const
    {
        return NearEqual(Type, other.Type) && NearEqual(propOpacity, other.propOpacity) &&
               NearEqual(propTranslate, other.propTranslate) && NearEqual(propScale, other.propScale) &&
               NearEqual(propRotate, other.propRotate);
    }
    std::string ToString() const
    {
        std::stringstream ss;
        ss << "type:"
           << (Type == TransitionType::ALL ? "all" : (Type == TransitionType::APPEARING ? "appear" : "disappear"))
           << ", opacity:" << (HasOpacity() ? std::to_string(GetOpacityValue()) : "none") << ", "
           << (HasTranslate() ? GetTranslate()->ToString() : "translate: none") << ", "
           << (HasScale() ? GetScale()->ToString() : "scale: none") << ", "
           << (HasRotate() ? GetRotate()->ToString() : "rotate: none");
        return ss.str();
    }
};

enum class ChainedTransitionEffectType {
    IDENTITY = 0,
    OPACITY,
    SLIDE_SWITCH,
    MOVE,
    TRANSLATE,
    ROTATE,
    SCALE,
    ASYMMETRIC,
};

enum class TransitionEdge {
    TOP = 0,
    BOTTOM,
    START,
    END,
};

class ChainedTransitionEffect : public AceType {
    DECLARE_ACE_TYPE(ChainedTransitionEffect, AceType);

public:
    explicit ChainedTransitionEffect(ChainedTransitionEffectType type) : type_(type) {}

    ChainedTransitionEffectType GetType() const
    {
        return type_;
    }
    const RefPtr<ChainedTransitionEffect>& GetNext() const
    {
        return next_;
    }
    const std::shared_ptr<AnimationOption>& GetAnimationOption() const
    {
        return animationOption_;
    }
    void SetNext(const RefPtr<ChainedTransitionEffect>& next)
    {
        next_ = next;
    }
    void SetAnimationOption(const std::shared_ptr<AnimationOption>& option)
    {
        animationOption_ = option;
    }
    virtual std::string ToString() = 0;

protected:
    std::string AnimationOptionToString() const
    {
        if (animationOption_) {
            return "{duration:" + std::to_string(animationOption_->GetDuration()) +
                   ", delay:" + std::to_string(animationOption_->GetDelay()) + ", curve:" +
                   (animationOption_->GetCurve() ? animationOption_->GetCurve()->ToString() : std::string("null")) +
                   "}";
        }
        return "null";
    }
    ChainedTransitionEffectType type_;
    std::shared_ptr<AnimationOption> animationOption_;
    RefPtr<ChainedTransitionEffect> next_;
};

class ChainedTranslateEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedTranslateEffect, ChainedTransitionEffect);

public:
    explicit ChainedTranslateEffect(const TranslateOptions& option)
        : ChainedTransitionEffect(ChainedTransitionEffectType::TRANSLATE), effect_(option)
    {}

    const TranslateOptions& GetEffect() const
    {
        return effect_;
    }
    std::string ToString() override
    {
        std::string ans = "{type: translate";
        ans += ", effect: " + effect_.ToString();
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }

private:
    TranslateOptions effect_;
};

class ChainedRotateEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedRotateEffect, ChainedTransitionEffect);

public:
    explicit ChainedRotateEffect(const RotateOptions& option)
        : ChainedTransitionEffect(ChainedTransitionEffectType::ROTATE), effect_(option)
    {}

    const RotateOptions& GetEffect() const
    {
        return effect_;
    }
    std::string ToString() override
    {
        std::string ans = "{type: rotate";
        ans += ", effect: " + effect_.ToString();
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }

private:
    RotateOptions effect_;
};

class ChainedScaleEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedScaleEffect, ChainedTransitionEffect);

public:
    explicit ChainedScaleEffect(const ScaleOptions& option)
        : ChainedTransitionEffect(ChainedTransitionEffectType::SCALE), effect_(option)
    {}

    const ScaleOptions& GetEffect() const
    {
        return effect_;
    }
    std::string ToString() override
    {
        std::string ans = "{type: scale";
        ans += ", effect: " + effect_.ToString();
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }

private:
    ScaleOptions effect_;
};

class ChainedOpacityEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedOpacityEffect, ChainedTransitionEffect);

public:
    explicit ChainedOpacityEffect(float opacity)
        : ChainedTransitionEffect(ChainedTransitionEffectType::OPACITY), opacity_(opacity)
    {}

    const float& GetEffect() const
    {
        return opacity_;
    }
    std::string ToString() override
    {
        std::string ans = "{type: opacity";
        ans += ", effect: " + std::to_string(opacity_);
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }

private:
    float opacity_;
};

class ChainedMoveEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedMoveEffect, ChainedTransitionEffect);

public:
    explicit ChainedMoveEffect(TransitionEdge edge)
        : ChainedTransitionEffect(ChainedTransitionEffectType::MOVE), edge_(edge)
    {}

    const TransitionEdge& GetEffect() const
    {
        return edge_;
    }
    std::string ToString() override
    {
        const static std::string edgeName[] = { "top", "bottom", "start", "end" };
        std::string ans = "{type: move";
        ans += ", effect: " + edgeName[static_cast<int>(edge_)];
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }

private:
    TransitionEdge edge_;
};

class ChainedSlideSwitchEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedSlideSwitchEffect, ChainedTransitionEffect);

public:
    explicit ChainedSlideSwitchEffect() : ChainedTransitionEffect(ChainedTransitionEffectType::SLIDE_SWITCH) {}
    std::string ToString() override
    {
        std::string ans = "{type: slideSwitch";
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }
};

class ChainedIdentityEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedIdentityEffect, ChainedTransitionEffect);

public:
    explicit ChainedIdentityEffect() : ChainedTransitionEffect(ChainedTransitionEffectType::IDENTITY) {}
    std::string ToString() override
    {
        std::string ans = "{type: identity";
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }
};

class ChainedAsymmetricEffect final : public ChainedTransitionEffect {
    DECLARE_ACE_TYPE(ChainedAsymmetricEffect, ChainedTransitionEffect);

public:
    explicit ChainedAsymmetricEffect(
        const RefPtr<ChainedTransitionEffect>& appear, const RefPtr<ChainedTransitionEffect>& disappear)
        : ChainedTransitionEffect(ChainedTransitionEffectType::ASYMMETRIC), appearEffect_(appear),
          disappearEffect_(disappear)
    {}

    const RefPtr<ChainedTransitionEffect>& GetAppearEffect() const
    {
        return appearEffect_;
    }
    const RefPtr<ChainedTransitionEffect>& GetDisappearEffect() const
    {
        return disappearEffect_;
    }
    std::string ToString() override
    {
        std::string ans = "{type: asymmetric";
        ans += ", effect: {appear: " + (appearEffect_ ? appearEffect_->ToString() : "null") +
               ", disappear: " + (disappearEffect_ ? disappearEffect_->ToString() : "null") + "}";
        ans += ", animation: " + AnimationOptionToString();
        ans += ", successor: " + (next_ ? next_->ToString() : std::string("null")) + "}";
        return ans;
    }

private:
    RefPtr<ChainedTransitionEffect> appearEffect_;
    RefPtr<ChainedTransitionEffect> disappearEffect_;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_COMPONENTS_NG_PROPERTIES_TRANSITION_PROPERTY_H
