#include"lottiedrawable.h"

/**
 * Plays the animation from the beginning. If speed is < 0, it will start at the end
 * and play towards the beginning
 */
void LottieDrawable::playAnimation()
{

}

/**
 * Continues playing the animation from its current position. If speed < 0, it will play backwards
 * from the current position.
 */
void LottieDrawable::resumeAnimation()
{

}

/**
 * Sets the minimum frame that the animation will start from when playing or looping.
 */
void LottieDrawable::setMinFrame(int startFrame)
{

}

/**
 * Sets the minimum progress that the animation will start from when playing or looping.
 */
void LottieDrawable::setMinProgress(float startProgress)
{

}

/**
 * Sets the maximum frame that the animation will end at when playing or looping.
 */
void LottieDrawable::setMaxFrame(int endFrame)
{

}

/**
 * Sets the maximum progress that the animation will end at when playing or looping.
 */
void LottieDrawable::setMaxProgress(float endProgress)
{

}

/**
 * @see #setMinFrame(int)
 * @see #setMaxFrame(int)
 */
void LottieDrawable::setMinAndMaxFrame(int minFrame, int maxFrame)
{
}

/**
 * @see #setMinProgress(float)
 * @see #setMaxProgress(float)
 */
void LottieDrawable::setMinAndMaxProgress(float minProgress, float maxProgress)
{

}

/**
 * Reverses the current animation speed. This does NOT play the animation.
 * @see #setSpeed(float)
 * @see #playAnimation()
 * @see #resumeAnimation()
 */
void LottieDrawable::reverseAnimationSpeed()
{

}

/**
 * Sets the playback speed. If speed < 0, the animation will play backwards.
 */
void LottieDrawable::setSpeed(float speed)
{

}

/**
 * Defines what this animation should do when it reaches the end. This
 * setting is applied only when the repeat count is either greater than
 * 0 or {@link LottieDrawable#INFINITE}. Defaults to LottieDrawable::RepeatMode::Restart.
 *
 * @param mode LottieDrawable::RepeatMode::Restart or LottieDrawable::RepeatMode::Reverse
 */
void LottieDrawable::setRepeatMode(LottieDrawable::RepeatMode mode)
{

}

/**
 * Defines what this animation should do when it reaches the end.
 *
 * @return either one of LottieDrawable::RepeatMode::Restart or LottieDrawable::RepeatMode::Reverse
 */
LottieDrawable::RepeatMode
LottieDrawable::getRepeatMode()
{
    return RepeatMode::Restart; //FIXME
}

/**
 * Sets how many times the animation should be repeated. If the repeat
 * count is 0, the animation is never repeated. If the repeat count is
 * greater than 0 or -1, the repeat mode will be taken
 * into account. The repeat count is 0 by default. if the repeat count is
 * -1 then the animation repeats for INFINITY.
 *
 * @param count the number of times the animation should be repeated
 */
void LottieDrawable::setRepeatCount(int count)
{
}

/**
 * Defines how many times the animation should repeat. The default value
 * is 0.
 *
 * @return the number of times the animation should repeat, or {@link LottieDrawable#INFINITE}
 */
int LottieDrawable::getRepeatCount()
{
    return 0; //FIXME
}

bool LottieDrawable::isAnimating()
{
    return false; //FIXME
}

void LottieDrawable::cancelAnimation()
{

}

void LottieDrawable::pauseAnimation()
{

}

/**
 * Sets the progress to the specified frame.
 * If the composition isn't set yet, the progress will be set to the frame when
 * it is.
 */
void LottieDrawable::setFrame(int frame)
{
}

/**
 * Get the currently rendered frame.
 */
int LottieDrawable::getFrame()
{
    return 0; //FIXME
}

void LottieDrawable::setProgress(float progress)
{
}

float LottieDrawable::getProgress()
{
    return 0; //FIXME
}

long LottieDrawable::getDuration()
{
  return 0;//composition != null ? composition.getDuration() : 0;
}

