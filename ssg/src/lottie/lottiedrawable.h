#ifndef LOTTIEDRAWABLE_H
#define LOTTIEDRAWABLE_H

class LottieComposition;
class LottieDrawable
{
public:
    enum class RepeatMode {
        Restart,
        Reverse
    };
    void playAnimation();
    void resumeAnimation();
    void setMinFrame(int startFrame);
    void setMinProgress(float startProgress);
    void setMaxFrame(int endFrame);
    void setMaxProgress(float endProgress);
    void setMinAndMaxFrame(int minFrame, int maxFrame);
    void setMinAndMaxProgress(float minProgress, float maxProgress);
    void reverseAnimationSpeed();
    void setSpeed(float speed);
    void setRepeatMode(LottieDrawable::RepeatMode mode);
    LottieDrawable::RepeatMode getRepeatMode();
    void setRepeatCount(int count);
    int getRepeatCount();
    bool isAnimating();
    void cancelAnimation();
    void pauseAnimation();
    void setFrame(int frame);
    int getFrame();
    void setProgress(float progress);
    float getProgress();
    long getDuration();
};

#endif // LOTTIEDRAWABLE_H
