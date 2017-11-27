#ifndef SGKEYFRAME_H
#define SGKEYFRAME_H
#include"sgglobal.h"
#include<vector>

template<typename T>
class KeyFrame
{
public:
    float getStartProgress();
    float getEndProgress();
    bool isStatic();
    bool containsProgress(float progress) {
      return progress >= getStartProgress() && progress < getEndProgress();
    }
private:
    T startValue;
    T endValue;
    float startFrame;
    float endFrame;
    //Interpolater interpolater;
};

/**
 * <K> Keyframe type
 * <A> Animation type
 */
template<typename K, typename A>
class BaseKeyframeAnimation
{
    BaseKeyframeAnimation(const std::vector<KeyFrame<K>> &keyframes);
    void setIsDiscrete();
    void setProgress(float progress);
    KeyFrame<K> getCurrentKeyframe();
    float getCurrentKeyframeProgress() {
        if (isDiscrete) {
          return 0;
        }

        KeyFrame<K> keyframe = getCurrentKeyframe();
        if (keyframe.isStatic()) {
          return 0;
        }
        float progressIntoFrame = progress - keyframe.getStartProgress();
        float keyframeProgress = keyframe.getEndProgress() - keyframe.getStartProgress();
        /**
         * This wil be [0, 1] unless the interpolator has overshoot in which case getValue() should be
         * able to handle values outside of that range.
         */
        //return keyframe.interpolator.getInterpolation(progressIntoFrame / keyframeProgress);
    }
    float getStartDelayProgress();
    float getEndProgress();
    A getValue() {
        return getValue(getCurrentKeyframe(), getCurrentKeyframeProgress());
      }

    float getProgress();
    /**
     * keyframeProgress will be [0, 1] unless the interpolator has overshoot in which case, this
     * should be able to handle values outside of that range.
     */
    virtual A getValue(KeyFrame<K> keyframe, float keyframeProgress) = 0;
private:
    std::vector<KeyFrame<K>> keyframes;
    KeyFrame<K> cachedKeyframe;
    float progress;
    bool  isDiscrete;
};

template<typename K, typename A> //interface
class AnimatableValue
{
  virtual BaseKeyframeAnimation<K, A> createAnimation() = 0;
  virtual bool hasAnimation() = 0;
};

template<typename K, typename O>
class BaseAnimatableValue<K, O> : public AnimatableValue<K, O>
{
public:
    BaseAnimatableValue(const std::vector<KeyFrame<K>> keyframes, K initialValue) {
      keyframes = keyframes;
      initialValue = initialValue;
    }
    /**
     * Convert the value type of the keyframe to the value type of the animation. Often, these
     * are the same type.
     */
    O convertType(K value) {
      //noinspection unchecked
      return (O) value;
    }
    bool hasAnimation() {
      return !keyframes.empty();
    }

    virtual O getInitialValue() {
      return convertType(initialValue);
    }
private:
  std::vector<KeyFrame<K>> keyframes;
  K initialValue;
};

class AnimatableIntegerValue : public BaseAnimatableValue<int, int>
{
public:
  AnimatableIntegerValue();
  BaseKeyframeAnimation<int, int> createAnimation() {
      if (!hasAnimation()) {
        return new StaticKeyframeAnimation<>(initialValue);
      }

      return new IntegerKeyframeAnimation(keyframes);
    }
};


#endif // SGKEYFRAME_H
