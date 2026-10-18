#include "presentation/sfml/SpriterAnimationPlayer.hpp"

#include "entity/entityinstance.h"
#include "model/spritermodel.h"
#include "sfml_implementation/examplefilefactory.h"
#include "sfml_implementation/exampleobjectfactory.h"
#include "sfml_implementation/sfmlimagefile.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <unordered_map>

namespace soma::presentation
{
    class SpriterAnimationPlayer::Implementation
    {
      public:
        Implementation(sf::RenderTarget& target, const std::filesystem::path& modelPath, const std::string& entityName)
            : model_(modelPath.string(),
                     new SpriterEngine::ExampleFileFactory(&target, &renderSettings_),
                     new SpriterEngine::ExampleObjectFactory(&target))
            , entity_(model_.getNewEntityInstance(entityName))
        {
            entity_->setCurrentAnimation("idle");
            static constexpr auto solidColorFragment = R"(
                uniform sampler2D texture;
                uniform vec4 replacement;
                void main()
                {
                    float alpha = texture2D(texture, gl_TexCoord[0].xy).a * gl_Color.a;
                    gl_FragColor = vec4(replacement.rgb, replacement.a * alpha);
                })";
            solidColorAvailable_ = solidColorShader_.loadFromMemory(solidColorFragment, sf::Shader::Type::Fragment);
            if (solidColorAvailable_)
            {
                solidColorShader_.setUniform("texture", sf::Shader::CurrentTexture);
            }
        }

        SpriterEngine::SfmlRenderSettings renderSettings_;
        sf::Shader solidColorShader_;
        bool solidColorAvailable_{};
        SpriterEngine::SpriterModel model_;
        std::unique_ptr<SpriterEngine::EntityInstance> entity_;
        AnimationBounds lastBounds_{};
        std::unordered_map<std::string, AnimationPoint> lastPoints_;
    };

    SpriterAnimationPlayer::SpriterAnimationPlayer(sf::RenderTarget& target, const std::filesystem::path& modelPath, std::string entityName)
        : implementation_(std::make_unique<Implementation>(target, modelPath, entityName))
    {
    }

    SpriterAnimationPlayer::~SpriterAnimationPlayer() = default;

    void SpriterAnimationPlayer::play(std::string_view animation, float speed)
    {
        implementation_->lastPoints_.clear();
        implementation_->entity_->setCurrentAnimation(std::string{animation});
        implementation_->entity_->setPlaybackSpeedRatio(speed);
        implementation_->entity_->setCurrentTime(0.0);
    }

    void SpriterAnimationPlayer::setPosition(float x, float y)
    {
        implementation_->entity_->setPosition({x, y});
        implementation_->entity_->reprocessCurrentTime();
    }

    void SpriterAnimationPlayer::setScale(float x, float y)
    {
        implementation_->entity_->setScale({x, y});
        implementation_->entity_->reprocessCurrentTime();
    }

    void SpriterAnimationPlayer::update(float elapsedMilliseconds)
    {
        implementation_->entity_->setTimeElapsed(elapsedMilliseconds);
    }

    void SpriterAnimationPlayer::render()
    {
        renderVisual();
        implementation_->entity_->playSoundTriggers();
    }

    void SpriterAnimationPlayer::renderVisual()
    {
        implementation_->entity_->render();
    }

    void SpriterAnimationPlayer::setTint(sf::Color color)
    {
        implementation_->renderSettings_.color = sf::Color::White;
        if (color == sf::Color::White)
        {
            implementation_->renderSettings_.shader = nullptr;
            return;
        }
        if (!implementation_->solidColorAvailable_)
        {
            implementation_->renderSettings_.color = color;
            return;
        }
        implementation_->solidColorShader_.setUniform(
            "replacement", sf::Glsl::Vec4{color.r / 255.0F, color.g / 255.0F, color.b / 255.0F, color.a / 255.0F});
        implementation_->renderSettings_.shader = &implementation_->solidColorShader_;
    }

    void SpriterAnimationPlayer::setRenderOffset(sf::Vector2f offset)
    {
        implementation_->renderSettings_.offset = offset;
    }

    AnimationBounds SpriterAnimationPlayer::bounds()
    {
        implementation_->entity_->reprocessCurrentTime();
        auto* box = implementation_->entity_->objectIfExistsOnCurrentFrame("bounding_box");
        if (box == nullptr)
        {
            box = implementation_->entity_->objectIfExistsOnCurrentFrame("hitbox");
        }
        if (box == nullptr)
        {
            return implementation_->lastBounds_;
        }
        const auto position = box->getPosition();
        const auto size = box->getSize();
        const auto scale = box->getScale();
        implementation_->lastBounds_ = {static_cast<float>(position.x),
                                        static_cast<float>(position.y),
                                        static_cast<float>(size.x * scale.x),
                                        static_cast<float>(size.y * scale.y)};
        return implementation_->lastBounds_;
    }

    std::optional<AnimationPoint> SpriterAnimationPlayer::point(std::string_view name)
    {
        implementation_->entity_->reprocessCurrentTime();
        const auto key = std::string{name};
        auto* object = implementation_->entity_->objectIfExistsOnCurrentFrame(key);
        if (object == nullptr)
        {
            const auto cached = implementation_->lastPoints_.find(key);
            return cached == implementation_->lastPoints_.end() ? std::nullopt : std::optional{cached->second};
        }
        const auto position = object->getPosition();
        const AnimationPoint result{static_cast<float>(position.x), static_cast<float>(position.y)};
        implementation_->lastPoints_.insert_or_assign(key, result);
        return result;
    }

    bool SpriterAnimationPlayer::isPlaying() const
    {
        return implementation_->entity_->animationIsPlaying();
    }

    std::string SpriterAnimationPlayer::currentAnimation() const
    {
        return implementation_->entity_->currentAnimationName();
    }
} // namespace soma::presentation
