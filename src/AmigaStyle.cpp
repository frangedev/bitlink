#include "AmigaStyle.h"

// Amiga-style palette: dark blue background, copper/amber accents
#define AMIGA_BG       0xff0f1528
#define AMIGA_PANEL    0xff1a2550
#define AMIGA_FACE     0xff3555aa
#define AMIGA_HIGH     0xff6a8acc
#define AMIGA_SHAD     0xff0a0f1a
#define AMIGA_TEXT     0xffffdd88
#define AMIGA_ACCENT   0xffffaa22

juce::Colour AmigaStyleLookAndFeel::background() { return juce::Colour(AMIGA_BG); }
juce::Colour AmigaStyleLookAndFeel::panel()     { return juce::Colour(AMIGA_PANEL); }
juce::Colour AmigaStyleLookAndFeel::buttonFace()    { return juce::Colour(AMIGA_FACE); }
juce::Colour AmigaStyleLookAndFeel::buttonHighlight() { return juce::Colour(AMIGA_HIGH); }
juce::Colour AmigaStyleLookAndFeel::buttonShadow()  { return juce::Colour(AMIGA_SHAD); }
juce::Colour AmigaStyleLookAndFeel::textColour() { return juce::Colour(AMIGA_TEXT); }
juce::Colour AmigaStyleLookAndFeel::accent()    { return juce::Colour(AMIGA_ACCENT); }

AmigaStyleLookAndFeel::AmigaStyleLookAndFeel()
{
    setDefaultSansSerifTypefaceName("Verdana");
}

void AmigaStyleLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour&,
                                                  bool highlighted, bool down)
{
    auto b = button.getLocalBounds().toFloat().reduced(0.5f);
    const float bevel = 2.0f;
    juce::Colour topLeft  = down ? buttonShadow() : buttonHighlight();
    juce::Colour botRight = down ? buttonHighlight() : buttonShadow();
    juce::Colour face = buttonFace();
    if (highlighted && !down)
        face = face.brighter(0.15f);

    g.setColour(face);
    g.fillRoundedRectangle(b.reduced(bevel), 2.0f);
    g.setColour(topLeft);
    g.drawLine(b.getX(), b.getBottom() - bevel, b.getX(), b.getY() + bevel, 1.2f);
    g.drawLine(b.getX() + bevel, b.getY(), b.getRight() - bevel, b.getY(), 1.2f);
    g.setColour(botRight);
    g.drawLine(b.getRight() - bevel, b.getY() + bevel, b.getRight() - bevel, b.getBottom(), 1.2f);
    g.drawLine(b.getX() + bevel, b.getBottom() - 0.5f, b.getRight() - bevel, b.getBottom() - 0.5f, 1.2f);
}

void AmigaStyleLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                          int, int, int buttonW, int buttonH, juce::ComboBox&)
{
    auto b = juce::Rectangle<float>(0.f, 0.f, (float)width, (float)height).reduced(0.5f);
    const float bevel = 1.5f;
    g.setColour(buttonFace());
    g.fillRoundedRectangle(b, 2.0f);
    g.setColour(buttonHighlight());
    g.drawLine(b.getX(), b.getBottom() - bevel, b.getX(), b.getY() + bevel, 1.0f);
    g.drawLine(b.getX() + bevel, b.getY(), b.getRight() - bevel, b.getY(), 1.0f);
    g.setColour(buttonShadow());
    g.drawLine(b.getRight() - bevel, b.getY() + bevel, b.getRight() - bevel, b.getBottom(), 1.0f);
    g.drawLine(b.getX() + bevel, b.getBottom() - 0.5f, b.getRight() - bevel, b.getBottom() - 0.5f, 1.0f);
}

void AmigaStyleLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                              bool highlighted, bool down)
{
    auto b = button.getLocalBounds().toFloat().reduced(1.0f);
    float boxSize = juce::jmin(b.getHeight(), 14.0f);
    auto box = juce::Rectangle<float>(b.getX(), b.getCentreY() - boxSize * 0.5f, boxSize, boxSize);
    g.setColour(buttonFace());
    g.fillRoundedRectangle(box, 2.0f);
    g.setColour(buttonShadow());
    g.drawRoundedRectangle(box, 2.0f, 1.0f);
    if (button.getToggleState())
    {
        g.setColour(accent());
        g.fillEllipse(box.reduced(3.0f));
    }
    g.setColour(textColour());
    g.setFont(juce::Font(11.0f));
    g.drawText(button.getButtonText(), box.getRight() + 4, 0, (int)(b.getWidth() - box.getWidth() - 4), (int)b.getHeight(), juce::Justification::centredLeft, true);
}

void AmigaStyleLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(textColour());
    g.setFont(label.getFont());
    g.drawFittedText(label.getText(), label.getLocalBounds().reduced(2), label.getJustificationType(), 1);
}

namespace BitLinkIcons
{
    static void folder(juce::Path& p)
    {
        p.clear();
        p.startNewSubPath(0.15f, 0.35f);
        p.lineTo(0.15f, 0.75f);
        p.lineTo(0.85f, 0.75f);
        p.lineTo(0.85f, 0.35f);
        p.lineTo(0.5f, 0.35f);
        p.lineTo(0.4f, 0.25f);
        p.lineTo(0.15f, 0.25f);
        p.closeSubPath();
    }
    juce::Path createLoadIcon() { juce::Path p; folder(p); return p; }

    static void clearX(juce::Path& p)
    {
        p.clear();
        p.startNewSubPath(0.2f, 0.2f); p.lineTo(0.8f, 0.8f);
        p.startNewSubPath(0.8f, 0.2f); p.lineTo(0.2f, 0.8f);
    }
    juce::Path createClearIcon() { juce::Path p; clearX(p); return p; }

    static void slices(juce::Path& p)
    {
        p.clear();
        p.addRectangle(0.1f, 0.5f, 0.15f, 0.35f);
        p.addRectangle(0.3f, 0.3f, 0.15f, 0.55f);
        p.addRectangle(0.5f, 0.45f, 0.15f, 0.4f);
        p.addRectangle(0.7f, 0.25f, 0.15f, 0.6f);
    }
    juce::Path createAutoSliceIcon() { juce::Path p; slices(p); return p; }

    static void norm(juce::Path& p)
    {
        p.clear();
        p.addRectangle(0.2f, 0.6f, 0.6f, 0.25f);
        p.addRectangle(0.35f, 0.35f, 0.3f, 0.5f);
    }
    juce::Path createNormIcon() { juce::Path p; norm(p); return p; }

    static void reverse(juce::Path& p)
    {
        p.clear();
        p.addTriangle(0.25f, 0.3f, 0.25f, 0.7f, 0.05f, 0.5f);
        p.addTriangle(0.75f, 0.3f, 0.75f, 0.7f, 0.95f, 0.5f);
    }
    juce::Path createReverseIcon() { juce::Path p; reverse(p); return p; }

    static void disk(juce::Path& p)
    {
        p.clear();
        p.addEllipse(0.2f, 0.2f, 0.6f, 0.6f);
        p.addEllipse(0.35f, 0.35f, 0.3f, 0.3f);
    }
    juce::Path createExport8SVXIcon() { return createExportWAVIcon(); }
    juce::Path createExportWAVIcon()
    {
        juce::Path p;
        p.startNewSubPath(0.1f, 0.5f);
        p.quadraticTo(0.3f, 0.2f, 0.5f, 0.5f);
        p.quadraticTo(0.7f, 0.8f, 0.9f, 0.5f);
        return p;
    }
    juce::Path createSavePresetIcon() { juce::Path p; disk(p); return p; }
    juce::Path createLoadPresetIcon() { juce::Path p; folder(p); return p; }
    juce::Path createSaveSoundIcon() { return createExportWAVIcon(); }
    juce::Path createLoadSoundIcon() { juce::Path p; folder(p); return p; }

    static void info(juce::Path& p)
    {
        p.clear();
        p.addEllipse(0.35f, 0.2f, 0.3f, 0.3f);
        p.addRectangle(0.42f, 0.55f, 0.16f, 0.35f);
    }
    juce::Path createInfoIcon() { juce::Path p; info(p); return p; }
}

void IconTextButton::paint(juce::Graphics& g)
{
    auto& laf = getLookAndFeel();
    laf.drawButtonBackground(g, *this,
                             findColour(juce::TextButton::buttonColourId),
                             isOver(), isDown());
    auto b = getLocalBounds().toFloat().reduced(2.0f);
    if (!iconPath.isEmpty())
    {
        float iconW = juce::jmin(b.getHeight() * 0.7f, b.getWidth() * 0.4f, 18.0f);
        auto iconRect = b.removeFromLeft(iconW + 2.0f).reduced(1.0f);
        iconRect = iconRect.withSizeKeepingCentre(iconW, iconW);
        g.setColour(findColour(juce::TextButton::textColourOffId));
        juce::Path p = iconPath;
        p.applyTransform(p.getTransformToScaleToFit(iconRect.reduced(1.0f), true));
        if (p.getBounds().getWidth() > 0 && p.getBounds().getHeight() > 0)
        {
            float strokeW = juce::jmin(1.5f, iconW * 0.12f);
            g.strokePath(p, juce::PathStrokeType(strokeW));
        }
        else
            g.fillPath(p);
    }
    if (getButtonText().isNotEmpty())
    {
        g.setColour(findColour(juce::TextButton::textColourOffId));
        g.setFont(juce::Font(10.0f));
        g.drawFittedText(getButtonText(), b.toNearestInt(), juce::Justification::centredLeft, 1);
    }
}
