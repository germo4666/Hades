// HADES by GERMO - interface. Everything is laid out on a 1200 x 860 canvas that scales with the window.
#include "PluginEditor.h"

namespace hd
{
namespace col
{
    const juce::Colour bg     { 0xff121314 }, panel { 0xff1b1c1e }, sel { 0xff25211f }, ink { 0xffede6d8 },
                       muted  { 0xff9c988f }, vis   { 0xff222326 }, track { 0xff2c2d30 }, ktrack { 0xff36373b },
                       border { 0xff2f3034 }, dim   { 0xff5a564f }, teal  { 0xff4fa39b }, eqBg { 0xff0d1817 },
                       eqGrid { 0xff1a2c2b }, eqBars{ 0xff173230 }, eqDiv { 0xff1e3533 }, eqText { 0xffb7c4bf },
                       accent { 0xffe5533a }, eqLine{ 0xfff08a6c };
}

static juce::Font mono (float h, bool bold = false)
{
#if JUCE_MAC
    const char* name = "Menlo";
#else
    const char* name = "DejaVu Sans Mono";
#endif
    return juce::Font (juce::FontOptions (name, h, bold ? juce::Font::bold : juce::Font::plain));
}
static juce::Font disp (float h)
{
#if JUCE_MAC
    return juce::Font (juce::FontOptions ("Helvetica Neue", h, juce::Font::bold));
#else
    return juce::Font (juce::FontOptions (h, juce::Font::bold));
#endif
}
static float textWidth (const juce::Font& f, const juce::String& s) { return juce::GlyphArrangement::getStringWidth (f, s); }

static void setParamReal (juce::RangedAudioParameter* p, float v) { if (p) p->setValueNotifyingHost (p->convertTo0to1 (v)); }
static float paramReal (juce::RangedAudioParameter* p) { return p ? p->convertFrom0to1 (p->getValue()) : 0.f; }

//==============================================================================
HadesLookAndFeel::HadesLookAndFeel()
{
    setColour (juce::PopupMenu::backgroundColourId, col::panel);
    setColour (juce::PopupMenu::textColourId, col::ink);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, col::accent);
    setColour (juce::PopupMenu::highlightedTextColourId, col::bg);
    setColour (juce::PopupMenu::headerTextColourId, col::muted);
}

juce::Font HadesLookAndFeel::getPopupMenuFont() { return mono (14.f); }

void HadesLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.f);
    const float r = std::min (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const bool big = (bool) s.getProperties()["big"];
    const bool bipolar = (bool) s.getProperties()["bipolar"];
    const juce::Colour arc = s.getProperties().contains ("arc") ? juce::Colour ((juce::uint32) (juce::int64) s.getProperties()["arc"]) : col::accent;
    const float thick = big ? r * 0.12f : r * 0.17f;
    const float rr = r - thick * 0.5f;
    const float alpha = s.isEnabled() ? 1.f : 0.35f;

    juce::Path track;
    track.addCentredArc (c.x, c.y, rr, rr, 0.f, a0, a1, true);
    g.setColour (col::ktrack.withMultipliedAlpha (alpha));
    g.strokePath (track, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float av = a0 + pos * (a1 - a0);
    const float from = bipolar ? (a0 + a1) * 0.5f : a0;
    if (std::abs (av - from) > 0.01f)
    {
        juce::Path vp;
        vp.addCentredArc (c.x, c.y, rr, rr, 0.f, std::min (from, av), std::max (from, av), true);
        g.setColour (arc.withMultipliedAlpha (alpha));
        g.strokePath (vp, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const float ir = rr - thick * 0.5f - (big ? r * 0.1f : r * 0.08f);
    if (big)
    {
        g.setColour (col::panel);
        g.fillEllipse (c.x - ir, c.y - ir, ir * 2, ir * 2);
        g.setColour (col::ink.withMultipliedAlpha (alpha));
        g.drawEllipse (c.x - ir, c.y - ir, ir * 2, ir * 2, 1.5f);
    }
    const juce::Point<float> tip (c.x + std::sin (av) * ir * 0.8f, c.y - std::cos (av) * ir * 0.8f);
    g.setColour (col::ink.withMultipliedAlpha (alpha));
    g.drawLine ({ c, tip }, big ? 3.f : 2.5f);
}

void HadesLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                         juce::Slider::SliderStyle style, juce::Slider& s)
{
    auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    if (style == juce::Slider::LinearBar)
    {
        r = r.reduced (0.75f);
        g.setColour (s.isMouseOverOrDragging() ? col::ink : col::ink.withAlpha (0.85f));
        g.drawRoundedRectangle (r, 8.f, 1.5f);
        g.setFont (mono (11.f));
        g.drawText (s.getProperties()["prefix"].toString() + s.getTextFromValue (s.getValue()), r, juce::Justification::centred);
        return;
    }
    // horizontal macro slider (AGE)
    const float cy = r.getCentreY();
    auto tr = juce::Rectangle<float> (r.getX() + 2, cy - 5, r.getWidth() - 4, 10);
    g.setColour (col::track); g.fillRoundedRectangle (tr, 5.f);
    g.setColour (col::accent); g.fillRoundedRectangle (tr.withRight (pos), 5.f);
    g.setColour (col::panel); g.fillEllipse (pos - 9, cy - 9, 18, 18);
    g.setColour (col::ink); g.drawEllipse (pos - 9, cy - 9, 18, 18, 2.f);
}

void HadesLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool hi, bool)
{
    if ((bool) b.getProperties()["plain"]) return;
    auto r = b.getLocalBounds().toFloat().reduced (0.75f);
    const bool on = b.getToggleState();
    if (on) { g.setColour (hi ? col::ink.brighter (0.1f) : col::ink); g.fillRoundedRectangle (r, 8.f); }
    else    { g.setColour (hi ? col::ink : col::ink.withAlpha (0.8f)); g.drawRoundedRectangle (r, 8.f, 1.5f); }
}

void HadesLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool hi, bool)
{
    const bool plain = (bool) b.getProperties()["plain"];
    g.setColour (b.getToggleState() && ! plain ? col::bg : (hi && plain ? col::accent : col::ink));
    g.setFont (plain ? mono (20.f) : mono (11.f, (bool) b.getProperties()["bold"]));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

//==============================================================================
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& s, const juce::String& id, const juce::String& lbl, bool big = false,
          juce::Colour arc = col::teal, bool bipolar = false, bool showText = true)
        : label (lbl), textVisible (showText)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        slider.getProperties().set ("big", big);
        slider.getProperties().set ("bipolar", bipolar);
        slider.getProperties().set ("arc", (juce::int64) arc.getARGB());
        slider.setMouseDragSensitivity (big ? 220 : 180);
        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, id, slider);
        if (auto* p = s.getParameter (id)) slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
        slider.onValueChange = [this] { repaint(); };
        addAndMakeVisible (slider);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        if (textVisible) b.removeFromBottom (28);
        const int s = std::min (b.getWidth(), b.getHeight());
        slider.setBounds (b.withSizeKeepingCentre (s, s));
    }
    void paint (juce::Graphics& g) override
    {
        if (! textVisible) return;
        auto b = getLocalBounds().removeFromBottom (28);
        g.setColour (col::ink);
        g.setFont (mono (10.f, true));
        g.drawText (label, b.removeFromTop (14), juce::Justification::centred);
        g.setColour (col::muted);
        g.setFont (mono (10.f));
        g.drawText (slider.getTextFromValue (slider.getValue()), b, juce::Justification::centred);
    }
    juce::Slider slider;

private:
    juce::String label;
    bool textVisible;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

//==============================================================================
class ChoiceChips : public juce::Component
{
public:
    ChoiceChips (juce::AudioProcessorValueTreeState& s, const juce::String& id, bool seg = false)
        : segmented (seg)
    {
        param = s.getParameter (id);
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (param)) labels = c->choices;
        att = std::make_unique<juce::ParameterAttachment> (*param, [this] (float v) { current = juce::roundToInt (v); repaint(); });
        att->sendInitialUpdate();
    }
    int preferredWidth() const
    {
        float w = segmented ? 6.f : 0.f;
        for (auto& l : labels) w += chipW (l) + gap();
        return (int) w;
    }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (segmented) { g.setColour (col::track); g.fillRoundedRectangle (b, 10.f); }
        rects.clear();
        float x = segmented ? 3.f : 0.f;
        for (int i = 0; i < labels.size(); ++i)
        {
            const float w = chipW (labels[i]);
            auto r = juce::Rectangle<float> (x, segmented ? 3.f : 0.f, w, b.getHeight() - (segmented ? 6.f : 0.f));
            rects.push_back (r);
            const bool on = (i == current);
            if (on) { g.setColour (col::ink); g.fillRoundedRectangle (r.reduced (0.75f), 7.f); }
            else if (! segmented) { g.setColour (col::ink.withAlpha (0.75f)); g.drawRoundedRectangle (r.reduced (0.75f), 7.f, 1.5f); }
            g.setColour (on ? col::bg : col::ink);
            g.setFont (mono (11.f));
            g.drawText (segmented ? labels[i].toUpperCase() : labels[i], r, juce::Justification::centred);
            x += w + gap();
        }
    }
    void mouseDown (const juce::MouseEvent& e) override
    {
        for (int i = 0; i < (int) rects.size(); ++i)
            if (rects[(size_t) i].contains (e.position)) { att->setValueAsCompleteGesture ((float) i); return; }
    }

private:
    float chipW (const juce::String& s) const { return textWidth (mono (11.f), segmented ? s.toUpperCase() : s) + (segmented ? 14.f : 20.f); }
    float gap() const { return segmented ? 2.f : 4.f; }
    juce::RangedAudioParameter* param = nullptr;
    juce::StringArray labels;
    int current = 0;
    bool segmented;
    std::vector<juce::Rectangle<float>> rects;
    std::unique_ptr<juce::ParameterAttachment> att;
};

//==============================================================================
class ToggleChip : public juce::TextButton
{
public:
    ToggleChip (juce::AudioProcessorValueTreeState& s, const juce::String& id, const juce::String& caption) : juce::TextButton (caption)
    {
        setClickingTogglesState (true);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, id, *this);
    }
private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att;
};

class DragValue : public juce::Slider
{
public:
    DragValue (juce::AudioProcessorValueTreeState& s, const juce::String& id, const juce::String& prefix)
    {
        setSliderStyle (juce::Slider::LinearBar);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        setSliderSnapsToMousePosition (false);
        getProperties().set ("prefix", prefix);
        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, id, *this);
        if (auto* p = s.getParameter (id)) setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
        setMouseDragSensitivity (300);
    }
private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

class LedButton : public juce::Button
{
public:
    LedButton (juce::AudioProcessorValueTreeState& s, const juce::String& id) : juce::Button (id)
    {
        setClickingTogglesState (true);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, id, *this);
        setTitle (id);
    }
    void paintButton (juce::Graphics& g, bool hi, bool) override
    {
        auto r = getLocalBounds().toFloat().withSizeKeepingCentre (13, 13);
        g.setColour (getToggleState() ? col::accent : col::dim);
        g.fillEllipse (r);
        g.setColour (hi ? col::ink : col::ink.withAlpha (0.85f));
        g.drawEllipse (r, 1.5f);
    }
private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att;
};

//==============================================================================
struct ModuleInfo { const char* name; const char* prefix; const char* modeId; const char* footId; const char* footLbl; };
static const ModuleInfo kModules[6] = {
    { "HEAT",  "heat",  "heatMode",  "heatMix",       "MIX" },
    { "TAPE",  "tape",  "tapeSpeed", "tapeMix",       "MIX" },
    { "DRIFT", "drift", "driftMode", "driftMix",      "MIX" },
    { "GLUE",  "glue",  "glueMode",  "glueMix",       "MIX" },
    { "GRAIN", "grain", "grainTime", "grainFeedback", "FB"  },
    { "SPACE", "space", "spaceMode", "spaceDecay",    "DCY" },
};

class ModuleCard : public juce::Component
{
public:
    ModuleCard (HadesProcessor& p, int idx, std::function<void (int)> select)
        : proc (p), index (idx), info (kModules[idx]), onSelect (std::move (select)),
          led (p.apvts, juce::String (info.prefix) + "On"),
          amt (p.apvts, juce::String (info.prefix) + "Amt", "", true, col::accent, false, false)
    {
        addAndMakeVisible (led);
        addAndMakeVisible (amt);
        modeP = p.apvts.getParameter (info.modeId);
        footP = p.apvts.getParameter (info.footId);
        fluxP = p.apvts.getParameter (juce::String (info.prefix) + "Flux");
        onP   = p.apvts.getParameter (juce::String (info.prefix) + "On");
        amtP  = p.apvts.getParameter (juce::String (info.prefix) + "Amt");
    }

    void setSelected (bool s) { if (s != selected) { selected = s; repaint(); } }

    void tick (float dt)
    {
        t += dt;
        if (index == 4) // grain particles
        {
            const bool on = paramReal (onP) > 0.5f;
            const float dens = paramReal (proc.apvts.getParameter ("grainDensity"));
            spawnAcc += dt * (on ? 4.f + dens * 26.f : 0.f);
            while (spawnAcc >= 1.f && particles.size() < 40)
            {
                spawnAcc -= 1.f;
                particles.push_back ({ rng.nextFloat(), 0.15f + 0.7f * rng.nextFloat(), 0.f, 1.5f + 3.5f * rng.nextFloat(), rng.nextFloat() < 0.3f });
            }
            for (auto& q : particles) { q.life += dt * 1.4f; q.x += dt * 0.05f; }
            particles.erase (std::remove_if (particles.begin(), particles.end(), [] (const Particle& q) { return q.life >= 1.f; }), particles.end());
        }
        repaint();
    }

    void resized() override
    {
        led.setBounds (getWidth() - 30, 10, 22, 22);
        amt.setBounds ((getWidth() - 96) / 2, 128, 96, 96);
    }

    void mouseDown (const juce::MouseEvent&) override { if (onSelect) onSelect (index); }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.75f);
        g.setColour (selected ? col::sel : col::panel);
        g.fillRoundedRectangle (b, 12.f);
        g.setColour (selected ? col::ink : col::border);
        g.drawRoundedRectangle (b, 12.f, 1.5f);

        g.setColour (col::muted); g.setFont (mono (10.f));
        g.drawText ("0" + juce::String (index + 1), 12, 14, 20, 16, juce::Justification::centredLeft);
        g.setColour (col::ink); g.setFont (disp (19.f));
        g.drawText (info.name, 30, 10, 120, 24, juce::Justification::centredLeft);

        auto vr = juce::Rectangle<float> (12.f, 40.f, (float) getWidth() - 24.f, 62.f);
        g.setColour (col::vis); g.fillRoundedRectangle (vr, 8.f);
        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (vr.toNearestInt());
            drawVisual (g, vr);
        }

        g.setColour (col::muted); g.setFont (mono (10.f));
        g.drawText (modeP ? modeP->getCurrentValueAsText().toUpperCase() : juce::String(), 12, 108, getWidth() - 24, 16, juce::Justification::centredLeft);

        const auto pct = [] (juce::RangedAudioParameter* p) { return juce::String (juce::roundToInt (paramReal (p) * 100.f)) + "%"; };
        const juce::String foot = index == 5 ? juce::String (paramReal (footP), 1) + "s" : pct (footP);
        auto fr = juce::Rectangle<int> (12, getHeight() - 34, getWidth() - 24, 16);
        g.setFont (mono (10.f));
        const int w3 = fr.getWidth() / 3;
        g.drawText ("AMT " + pct (amtP), fr.removeFromLeft (w3), juce::Justification::centredLeft);
        g.drawText (juce::String (info.footLbl) + " " + foot, fr.removeFromLeft (w3), juce::Justification::centred);
        g.drawText ("FLX " + pct (fluxP), fr, juce::Justification::centredRight);
    }

private:
    struct Particle { float x, y, life, size; bool hot; };

    void drawVisual (juce::Graphics& g, juce::Rectangle<float> r)
    {
        const bool on = paramReal (onP) > 0.5f;
        const float a = on ? 1.f : 0.35f;
        const float amount = paramReal (amtP);
        const float cx = r.getCentreX(), cy = r.getCentreY();
        switch (index)
        {
            case 0: // HEAT: filament in a bulb, glows with drive
            {
                const float lvl = proc.getHeatLevel();
                juce::Path bulb;
                bulb.startNewSubPath (cx - 50, cy + 16); bulb.lineTo (cx - 50, cy);
                bulb.quadraticTo (cx - 50, cy - 19, cx, cy - 19); bulb.quadraticTo (cx + 50, cy - 19, cx + 50, cy);
                bulb.lineTo (cx + 50, cy + 16);
                g.setColour (col::dim.withMultipliedAlpha (a)); g.strokePath (bulb, juce::PathStrokeType (1.5f));
                g.setColour (col::ink.withMultipliedAlpha (a)); g.drawLine (cx - 62, cy + 20, cx + 62, cy + 20, 1.5f);
                juce::Path fil;
                for (int i = 0; i <= 10; ++i)
                {
                    const float x = cx - 38 + i * 7.6f;
                    const float jit = on ? std::sin (t * 23.f + (float) i) * lvl * 2.f : 0.f;
                    const float y = cy + ((i % 2) ? -8.f : 8.f) + jit;
                    if (i == 0) fil.startNewSubPath (x, y); else fil.lineTo (x, y);
                }
                const float glow = on ? 0.35f + 0.65f * std::max (amount, lvl) : 0.f;
                g.setColour (col::accent.withAlpha (0.18f * glow)); g.strokePath (fil, juce::PathStrokeType (7.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour (col::accent.withMultipliedAlpha (on ? 0.6f + 0.4f * glow : 0.35f));
                g.strokePath (fil, juce::PathStrokeType (2.5f, juce::PathStrokeType::mitered));
                break;
            }
            case 1: // TAPE: two reels turning at tape speed
            {
                const float speed = on ? 1.5f * (float) (1 << juce::roundToInt (paramReal (modeP))) : 0.f;
                const float ang = t * speed;
                const juce::Point<float> c1 (cx - 32, cy - 2), c2 (cx + 32, cy - 2);
                for (int k = 0; k < 2; ++k)
                {
                    const auto c = k == 0 ? c1 : c2;
                    const float rr = k == 0 ? 20.f : 15.f;
                    g.setColour (col::ink.withMultipliedAlpha (a));
                    g.drawEllipse (c.x - rr, c.y - rr, rr * 2, rr * 2, 2.f);
                    g.fillEllipse (c.x - 4.5f, c.y - 4.5f, 9, 9);
                    for (int s = 0; s < 3; ++s)
                    {
                        const float an = ang * (k == 0 ? 1.f : 1.33f) + s * juce::MathConstants<float>::twoPi / 3.f;
                        g.drawLine (c.x + std::sin (an) * 7, c.y - std::cos (an) * 7, c.x + std::sin (an) * (rr - 4), c.y - std::cos (an) * (rr - 4), 2.f);
                    }
                }
                juce::Path tp;
                tp.startNewSubPath (c1.x, c1.y + 20); tp.lineTo (cx, cy + 26); tp.lineTo (c2.x, c2.y + 15);
                g.setColour (col::accent.withMultipliedAlpha (a)); g.strokePath (tp, juce::PathStrokeType (2.5f));
                break;
            }
            case 2: // DRIFT: wavering lines
            {
                const float rate = paramReal (proc.apvts.getParameter ("driftRate"));
                const float amp = (on ? 4.f + 16.f * amount : 2.f) + proc.getDriftVis() * 4.f;
                for (int k = 0; k < 2; ++k)
                {
                    juce::Path wv;
                    for (int i = 0; i <= 60; ++i)
                    {
                        const float x = r.getX() + 6 + (r.getWidth() - 12) * i / 60.f;
                        const float ph = (float) i / 60.f * 9.5f + t * (0.8f + rate * 3.f) * (on ? 1.f : 0.f) + k * 0.6f;
                        const float y = cy + std::sin (ph) * amp * (k == 0 ? 1.f : 0.8f);
                        if (i == 0) wv.startNewSubPath (x, y); else wv.lineTo (x, y);
                    }
                    g.setColour ((k == 0 ? col::accent : col::teal).withMultipliedAlpha (a * (k == 0 ? 1.f : 0.6f)));
                    g.strokePath (wv, juce::PathStrokeType (k == 0 ? 2.5f : 1.5f));
                }
                break;
            }
            case 3: // GLUE: gain-reduction needle
            {
                const juce::Point<float> base (cx, r.getBottom() - 6);
                const float R = 46.f;
                juce::Path arcP; arcP.addCentredArc (base.x, base.y, R, R, 0.f, -1.1f, 1.1f, true);
                g.setColour (col::ink.withMultipliedAlpha (a)); g.strokePath (arcP, juce::PathStrokeType (1.5f));
                juce::Path hot; hot.addCentredArc (base.x, base.y, R - 2, R - 2, 0.f, 0.6f, 1.1f, true);
                g.setColour (col::accent.withMultipliedAlpha (a)); g.strokePath (hot, juce::PathStrokeType (4.f));
                for (float tk : { -0.8f, 0.f, 0.8f })
                    g.drawLine (base.x + std::sin (tk) * (R - 7), base.y - std::cos (tk) * (R - 7), base.x + std::sin (tk) * R, base.y - std::cos (tk) * R, 1.5f);
                needle += 0.35f * (juce::jlimit (-18.f, 0.f, proc.getGlueGr()) - needle);
                const float ang = 1.0f + (needle / 18.f) * 2.0f;
                g.setColour (col::ink.withMultipliedAlpha (a));
                g.drawLine (base.x, base.y, base.x + std::sin (ang) * (R - 6), base.y - std::cos (ang) * (R - 6), 2.f);
                g.fillEllipse (base.x - 4, base.y - 4, 8, 8);
                break;
            }
            case 4: // GRAIN: particles
            {
                if (particles.empty())
                {
                    g.setColour (col::teal.withMultipliedAlpha (0.35f));
                    for (int i = 0; i < 8; ++i) g.fillEllipse (r.getX() + 20 + i * 18.f, cy + std::sin ((float) i * 1.7f) * 12.f - 2, 4, 4);
                }
                for (auto& q : particles)
                {
                    const float al = std::sin (q.life * juce::MathConstants<float>::pi);
                    const float px = r.getX() + 8 + std::fmod (q.x, 1.f) * (r.getWidth() - 16), py = r.getY() + q.y * r.getHeight();
                    g.setColour ((q.hot ? col::accent : col::teal).withAlpha (al));
                    g.fillEllipse (px - q.size, py - q.size, q.size * 2, q.size * 2);
                }
                break;
            }
            default: // SPACE: rings
            {
                const float lvl = proc.getSpaceLevel();
                const float speed = on ? 0.25f + 0.3f * amount : 0.f;
                for (int k = 0; k < 5; ++k)
                {
                    const float ph = std::fmod ((float) k / 5.f + t * speed, 1.f);
                    const float rr = 6.f + ph * 56.f;
                    g.setColour (col::teal.withAlpha ((1.f - ph) * 0.8f * a));
                    g.drawEllipse (cx - rr, cy - rr, rr * 2, rr * 2, 2.f);
                }
                const float d = 4.f + lvl * 3.f;
                g.setColour (col::accent.withMultipliedAlpha (a)); g.fillEllipse (cx - d, cy - d, d * 2, d * 2);
                break;
            }
        }
    }

    HadesProcessor& proc;
    int index;
    ModuleInfo info;
    std::function<void (int)> onSelect;
    LedButton led;
    Knob amt;
    juce::RangedAudioParameter *modeP = nullptr, *footP = nullptr, *fluxP = nullptr, *onP = nullptr, *amtP = nullptr;
    bool selected = false;
    float t = 0, needle = 0, spawnAcc = 0;
    std::vector<Particle> particles;
    juce::Random rng;
};

//==============================================================================
class ModuleDrawer : public juce::Component
{
public:
    struct K { const char* id; const char* label; bool bipolar = false; };
    struct T { const char* id; const char* label; };

    ModuleDrawer (juce::AudioProcessorValueTreeState& s, const juce::String& t, const char* chipId,
                  std::vector<K> knobs, std::vector<T> toggles, const char* chip2Id = nullptr, const char* chip2Caption = nullptr)
        : title (t), chip2Label (chip2Caption ? chip2Caption : "")
    {
        chips = std::make_unique<ChoiceChips> (s, chipId);
        addAndMakeVisible (*chips);
        if (chip2Id) { chips2 = std::make_unique<ChoiceChips> (s, chip2Id); addAndMakeVisible (*chips2); }
        for (auto& k : knobs) { auto* kn = knobsOwned.add (new Knob (s, k.id, k.label, false, col::teal, k.bipolar)); addAndMakeVisible (kn); }
        for (auto& tg : toggles) { auto* b = toggleOwned.add (new ToggleChip (s, tg.id, tg.label)); addAndMakeVisible (b); }
    }
    void paint (juce::Graphics& g) override
    {
        g.setColour (col::ink); g.setFont (disp (21.f));
        g.drawText (title, 0, 0, 120, 32, juce::Justification::centredLeft);
        if (chips2)
        {
            g.setColour (col::muted); g.setFont (mono (10.f));
            g.drawText (chip2Label, chips2->getX() - 44, chips2->getY(), 40, chips2->getHeight(), juce::Justification::centredRight);
        }
    }
    void resized() override
    {
        int x = (int) textWidth (disp (21.f), title) + 16;
        chips->setBounds (x, 2, chips->preferredWidth(), 28);
        x += chips->preferredWidth() + 14;
        for (auto* b : toggleOwned)
        {
            const int w = (int) textWidth (mono (11.f), b->getButtonText()) + 22;
            b->setBounds (x, 2, w, 28); x += w + 6;
        }
        if (chips2) { x += 48; chips2->setBounds (x, 2, chips2->preferredWidth(), 28); }

        const int kw = 80, kh = 84;
        int kx = 0, ky = 50;
        for (auto* k : knobsOwned)
        {
            if (kx + kw > getWidth()) { kx = 0; ky += kh + 6; }
            k->setBounds (kx, ky, kw, kh); kx += kw;
        }
    }

private:
    juce::String title, chip2Label;
    std::unique_ptr<ChoiceChips> chips, chips2;
    juce::OwnedArray<Knob> knobsOwned;
    juce::OwnedArray<ToggleChip> toggleOwned;
};

//==============================================================================
class FluxPanel : public juce::Component
{
public:
    explicit FluxPanel (juce::AudioProcessorValueTreeState& s)
        : rate (s, "fluxRate", "RATE", false, col::accent), chaos (s, "fluxChaos", "CHAOS", false, col::accent),
          rateP (s.getParameter ("fluxRate")), chaosP (s.getParameter ("fluxChaos"))
    {
        for (auto& m : kModules)
        {
            auto* k = depth.add (new Knob (s, juce::String (m.prefix) + "Flux", "DEPTH", false, col::accent));
            addChildComponent (k);
            depthP.push_back (s.getParameter (juce::String (m.prefix) + "Flux"));
        }
        addAndMakeVisible (rate); addAndMakeVisible (chaos);
        walk.seed (5);
        hist.fill (0.f);
    }
    void setModule (int i) { for (int k = 0; k < depth.size(); ++k) depth[k]->setVisible (k == i); current = i; repaint(); }
    void tick (float dt)
    {
        walk.setRate (0.3f + paramReal (rateP) * 4.f, 1.f / std::max (dt, 0.001f));
        const float v = walk.next (paramReal (chaosP)) * (0.15f + 0.85f * paramReal (depthP[(size_t) current]));
        hist[(size_t) histPos] = v; histPos = (histPos + 1) % (int) hist.size();
        repaint();
    }
    void paint (juce::Graphics& g) override
    {
        g.setColour (col::ink); g.setFont (disp (17.f));
        g.drawText ("FLUX", 0, 0, 80, 26, juce::Justification::centredLeft);
        g.setColour (col::muted); g.setFont (mono (10.f));
        g.drawText (juce::String (juce::CharPointer_UTF8 ("random walk \xc2\xb7 linked L/R 85%")), 70, 0, getWidth() - 70, 26, juce::Justification::centredRight);
        auto cr = juce::Rectangle<float> (0, 36, (float) getWidth(), 44);
        juce::Path p;
        const int n = (int) hist.size();
        for (int i = 0; i < n; ++i)
        {
            const float v = hist[(size_t) ((histPos + i) % n)];
            const float x = cr.getX() + cr.getWidth() * i / (n - 1.f), y = cr.getCentreY() - v * cr.getHeight() * 0.45f;
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (col::accent); g.strokePath (p, juce::PathStrokeType (2.f));
        g.setColour (col::ink.withAlpha (0.6f));
        const float px = cr.getRight() - 1;
        for (float y = cr.getY(); y < cr.getBottom(); y += 5) g.drawLine (px, y, px, y + 2, 1.f);
    }
    void resized() override
    {
        const int w = getWidth() / 3;
        for (auto* k : depth) k->setBounds (0, 96, w, 84);
        rate.setBounds (w, 96, w, 84);
        chaos.setBounds (2 * w, 96, w, 84);
    }

private:
    juce::OwnedArray<Knob> depth;
    Knob rate, chaos;
    juce::RangedAudioParameter *rateP, *chaosP;
    std::vector<juce::RangedAudioParameter*> depthP;
    SmoothRandom walk;
    std::array<float, 120> hist {};
    int histPos = 0, current = 0;
};

//==============================================================================
class EqPanel : public juce::Component
{
public:
    EqPanel (HadesProcessor& p) : proc (p), prePost (p.apvts, "eqPost"), onLed (p.apvts, "eqOn"), fft (11)
    {
        addAndMakeVisible (prePost); addAndMakeVisible (onLed);
        for (auto* id : { "hpFreq", "lowGain", "lowFreq", "midGain", "midFreq", "midQ", "highGain", "highFreq", "eqOn" })
            prm.push_back (p.apvts.getParameter (id));
        bars.fill (0.f);
    }

    void resized() override
    {
        prePost.setBounds (16, getHeight() - 40, prePost.preferredWidth(), 26);
        onLed.setBounds (78, 10, 22, 22);
    }

    void tick()
    {
        // spectrum of the output
        constexpr int N = 2048;
        std::array<float, N * 2> data {};
        const int w = proc.scopeWrite.load();
        for (int i = 0; i < N; ++i)
        {
            const float win = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * i / (N - 1));
            data[(size_t) i] = proc.scope[(size_t) ((w - N + i) & 4095)] * win;
        }
        fft.performFrequencyOnlyForwardTransform (data.data());
        const int nb = (int) bars.size();
        for (int b = 0; b < nb; ++b)
        {
            const float f0 = 20.f * std::pow (1000.f, (float) b / nb), f1 = 20.f * std::pow (1000.f, (float) (b + 1) / nb);
            const int i0 = juce::jlimit (1, N / 2 - 1, (int) (f0 / 48000.f * N)), i1 = juce::jlimit (i0 + 1, N / 2, (int) (f1 / 48000.f * N));
            float m = 0;
            for (int i = i0; i < i1; ++i) m = std::max (m, data[(size_t) i]);
            const float db = gainToDb (m / (N * 0.25f)) + 4.5f * std::log2 ((f0 + f1) * 0.5f / 1000.f) * 0.5f; // gentle slope comp
            const float v = juce::jlimit (0.f, 1.f, (db + 78.f) / 72.f);
            bars[(size_t) b] = v > bars[(size_t) b] ? v : bars[(size_t) b] * 0.9f + v * 0.1f;
        }
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (col::eqBg); g.fillRoundedRectangle (b, 12.f);
        g.setColour (col::eqDiv); g.drawLine (110, 0, 110, b.getHeight(), 1.f);
        g.setColour (col::ink); g.setFont (mono (12.f, true));
        g.drawText ("SOFT EQ", 16, 12, 70, 18, juce::Justification::centredLeft);
        g.setColour (col::eqText); g.setFont (mono (10.f));
        g.drawText (juce::CharPointer_UTF8 ("\xc2\xb1" "12 dB"), 16, 32, 80, 14, juce::Justification::centredLeft);

        const auto gr = graph();
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (gr.toNearestInt());

        // spectrum
        const int nb = (int) bars.size();
        const float bw = gr.getWidth() / nb;
        g.setColour (col::eqBars);
        for (int i = 0; i < nb; ++i)
        {
            const float h = bars[(size_t) i] * gr.getHeight() * 0.75f;
            g.fillRect (gr.getX() + i * bw + 2, gr.getBottom() - h, bw - 4, h);
        }
        // grid
        g.setColour (col::eqGrid);
        g.drawLine (gr.getX(), gainToY (0), gr.getRight(), gainToY (0), 1.f);
        g.setColour (col::eqText.withAlpha (0.7f)); g.setFont (mono (10.f));
        for (float f : { 100.f, 1000.f, 10000.f })
        {
            const float x = freqToX (f);
            g.setColour (col::eqGrid); g.drawLine (x, gr.getY(), x, gr.getBottom(), 1.f);
            g.setColour (col::eqText.withAlpha (0.7f));
            g.drawText (f >= 1000 ? juce::String ((int) (f / 1000)) + "k" : juce::String ((int) f), (int) x + 4, (int) gr.getBottom() - 16, 40, 14, juce::Justification::centredLeft);
        }

        // response curve
        const bool on = paramReal (prm[8]) > 0.5f;
        juce::Path curve;
        for (int i = 0; i <= 240; ++i)
        {
            const float x = gr.getX() + gr.getWidth() * i / 240.f;
            const float y = gainToY (responseDb (xToFreq (x)));
            if (i == 0) curve.startNewSubPath (x, y); else curve.lineTo (x, y);
        }
        g.setColour (col::eqLine.withAlpha (on ? 1.f : 0.35f));
        g.strokePath (curve, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // handles + labels
        g.setFont (mono (10.f));
        for (int k = 0; k < 4; ++k)
        {
            const auto h = handlePos (k);
            g.setColour (col::ink.withAlpha (on ? 1.f : 0.5f));
            g.fillEllipse (h.x - 8, h.y - 8, 16, 16);
            g.setColour (col::eqBg); g.drawEllipse (h.x - 8, h.y - 8, 16, 16, 2.f);
            g.setColour (col::eqText);
            const auto txt = handleLabel (k);
            const float tw = textWidth (mono (10.f), txt) + 4;
            float tx = h.x + 12, ty = h.y < gr.getCentreY() ? h.y + 10 : h.y - 24;
            if (tx + tw > gr.getRight()) tx = h.x - 12 - tw;
            g.drawText (txt, (int) tx, (int) ty, (int) tw, 14, juce::Justification::centredLeft);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        drag = -1;
        float best = 18.f;
        for (int k = 0; k < 4; ++k) { const float d = handlePos (k).getDistanceFrom (e.position); if (d < best) { best = d; drag = k; } }
        if (drag >= 0) for (auto* p : dragParams (drag)) p->beginChangeGesture();
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (drag < 0) return;
        const auto gr = graph();
        const float f = xToFreq (juce::jlimit (gr.getX(), gr.getRight(), e.position.x));
        const float db = juce::jlimit (-12.f, 12.f, yToGain (e.position.y));
        switch (drag)
        {
            case 0: setParamReal (prm[0], juce::jlimit (20.f, 600.f, f)); break;
            case 1: setParamReal (prm[2], juce::jlimit (30.f, 500.f, f)); setParamReal (prm[1], db); break;
            case 2: setParamReal (prm[4], juce::jlimit (150.f, 8000.f, f)); setParamReal (prm[3], db); break;
            default: setParamReal (prm[7], juce::jlimit (1500.f, 18000.f, f)); setParamReal (prm[6], db); break;
        }
    }
    void mouseUp (const juce::MouseEvent&) override
    {
        if (drag >= 0) for (auto* p : dragParams (drag)) p->endChangeGesture();
        drag = -1;
    }
    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        for (int k = 0; k < 4; ++k)
            if (handlePos (k).getDistanceFrom (e.position) < 18.f)
                for (auto* p : dragParams (k)) p->setValueNotifyingHost (p->getDefaultValue());
    }
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
    {
        if (handlePos (2).getDistanceFrom (e.position) < 30.f)
            setParamReal (prm[5], juce::jlimit (0.3f, 3.f, paramReal (prm[5]) * (w.deltaY > 0 ? 1.1f : 0.91f)));
    }

private:
    juce::Rectangle<float> graph() const { return getLocalBounds().toFloat().withTrimmedLeft (111.f).withTrimmedRight (1.f); }
    float freqToX (float f) const { const auto g = graph(); return g.getX() + g.getWidth() * std::log (f / 20.f) / std::log (1000.f); }
    float xToFreq (float x) const { const auto g = graph(); return 20.f * std::pow (1000.f, (x - g.getX()) / g.getWidth()); }
    float gainToY (float db) const { const auto g = graph(); return g.getCentreY() - db / 15.f * g.getHeight() * 0.45f; }
    float yToGain (float y) const { const auto g = graph(); return (g.getCentreY() - y) / (g.getHeight() * 0.45f) * 15.f; }

    float responseDb (float f) const
    {
        constexpr double sr = 48000.0;
        Biquad hp, ls, pk, hs;
        const float hpF = paramReal (prm[0]);
        double m = 1.0;
        if (hpF > 21.f) { hp.highpass (sr, hpF, 0.707); m *= hp.magnitude (f, sr); }
        ls.lowShelf (sr, paramReal (prm[2]), 0.55, paramReal (prm[1]));
        pk.peak (sr, paramReal (prm[4]), paramReal (prm[5]), paramReal (prm[3]));
        hs.highShelf (sr, paramReal (prm[7]), 0.55, paramReal (prm[6]));
        m *= ls.magnitude (f, sr) * pk.magnitude (f, sr) * hs.magnitude (f, sr);
        return juce::jlimit (-30.f, 30.f, (float) (20.0 * std::log10 (std::max (m, 1.0e-6))));
    }
    juce::Point<float> handlePos (int k) const
    {
        switch (k)
        {
            case 0: { const float f = std::max (paramReal (prm[0]), 24.f); return { freqToX (f), gainToY (responseDb (f)) }; }
            case 1: return { freqToX (paramReal (prm[2])), gainToY (paramReal (prm[1])) };
            case 2: return { freqToX (paramReal (prm[4])), gainToY (paramReal (prm[3])) };
            default: return { freqToX (paramReal (prm[7])), gainToY (paramReal (prm[6])) };
        }
    }
    juce::String handleLabel (int k) const
    {
        const auto hz = [] (float f) { return f >= 1000.f ? juce::String (f / 1000.f, 1) + " kHz" : juce::String (juce::roundToInt (f)) + " Hz"; };
        const auto db = [] (float d) { return (d > 0.05f ? "+" : "") + juce::String (d, 1) + " dB"; };
        const juce::String dot = juce::CharPointer_UTF8 (" \xc2\xb7 ");
        switch (k)
        {
            case 0: return paramReal (prm[0]) > 21.f ? "HP " + hz (paramReal (prm[0])) : juce::String ("HP OFF");
            case 1: return "LOW " + db (paramReal (prm[1])) + dot + hz (paramReal (prm[2]));
            case 2: return "MID " + db (paramReal (prm[3])) + dot + hz (paramReal (prm[4])) + dot + "Q " + juce::String (paramReal (prm[5]), 1);
            default: return "HIGH " + db (paramReal (prm[6])) + dot + hz (paramReal (prm[7]));
        }
    }
    std::vector<juce::RangedAudioParameter*> dragParams (int k) const
    {
        switch (k) { case 0: return { prm[0] }; case 1: return { prm[1], prm[2] }; case 2: return { prm[3], prm[4] }; default: return { prm[6], prm[7] }; }
    }

    HadesProcessor& proc;
    ChoiceChips prePost;
    LedButton onLed;
    std::vector<juce::RangedAudioParameter*> prm;
    juce::dsp::FFT fft;
    std::array<float, 56> bars {};
    int drag = -1;
};

//==============================================================================
class PresetBox : public juce::Component
{
public:
    explicit PresetBox (HadesProcessor& p) : proc (p)
    {
        for (auto* b : { &prev, &next }) { b->getProperties().set ("plain", true); addAndMakeVisible (*b); }
        prev.onClick = [this] { step (-1); };
        next.onClick = [this] { step (1); };
    }
    void step (int d)
    {
        const int n = proc.getNumPrograms();
        proc.loadPreset ((proc.getPresetIndex() + d + n) % n);
        repaint();
    }
    void refresh() { if (shown != proc.getPresetIndex()) { shown = proc.getPresetIndex(); repaint(); } }
    void resized() override { prev.setBounds (6, 7, 30, getHeight() - 14); next.setBounds (getWidth() - 36, 7, 30, getHeight() - 14); }
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.75f);
        g.setColour (col::panel); g.fillRoundedRectangle (b, b.getHeight() * 0.5f);
        g.setColour (col::ink); g.drawRoundedRectangle (b, b.getHeight() * 0.5f, 1.5f);
        const auto& ps = factoryPresets();
        const int i = proc.getPresetIndex();
        const auto& pr = ps[(size_t) juce::jlimit (0, (int) ps.size() - 1, i)];
        const juce::String num = juce::String (i + 1).paddedLeft ('0', 2) + "/" + juce::String ((int) ps.size());
        g.setColour (col::muted); g.setFont (mono (10.f));
        g.drawText (juce::String (pr.cat).toUpperCase() + juce::String (juce::CharPointer_UTF8 (" \xc2\xb7 ")) + num, 44, 6, getWidth() - 88, 14, juce::Justification::centredLeft);
        g.setColour (col::ink); g.setFont (mono (14.f, true));
        g.drawText (pr.name, 44, 20, getWidth() - 88, 20, juce::Justification::centredLeft, true);
    }
    void mouseDown (const juce::MouseEvent&) override
    {
        juce::PopupMenu menu;
        const auto& ps = factoryPresets();
        juce::String cat; juce::PopupMenu sub;
        for (int i = 0; i < (int) ps.size(); ++i)
        {
            if (cat != ps[(size_t) i].cat)
            {
                if (cat.isNotEmpty()) menu.addSubMenu (cat, sub);
                sub = juce::PopupMenu(); cat = ps[(size_t) i].cat;
            }
            sub.addItem (i + 1, ps[(size_t) i].name, true, i == proc.getPresetIndex());
        }
        menu.addSubMenu (cat, sub);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                            [this] (int r) { if (r > 0) { proc.loadPreset (r - 1); repaint(); } });
    }

private:
    HadesProcessor& proc;
    juce::TextButton prev { juce::CharPointer_UTF8 ("\xe2\x80\xb9") }, next { juce::CharPointer_UTF8 ("\xe2\x80\xba") };
    int shown = -1;
};

//==============================================================================
class LuButton : public juce::Button
{
public:
    explicit LuButton (HadesProcessor& p) : juce::Button ("LU MATCH"), proc (p)
    {
        setClickingTogglesState (true);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "luMatch", *this);
        setTooltip ("Loudness match: keeps the output as loud as the input so you judge tone, not volume");
    }
    void paintButton (juce::Graphics& g, bool hi, bool) override
    {
        const bool on = getToggleState();
        auto b = getLocalBounds().toFloat().reduced (0.75f);
        g.setColour (on ? col::accent : (hi ? col::ink : col::ktrack));
        g.drawRoundedRectangle (b, 8.f, 1.5f);
        g.setColour (on ? col::accent : col::ktrack);
        g.fillEllipse (12, b.getCentreY() - 4, 8, 8);
        g.setColour (col::ink); g.setFont (mono (10.5f, true));
        g.drawText ("LU MATCH", 25, 0, 62, getHeight(), juce::Justification::centredLeft);
        g.setColour (col::muted); g.setFont (mono (11.f));
        const float d = proc.getLuDelta();
        const juce::String v = on ? juce::String (juce::CharPointer_UTF8 ("\xce\x94 ")) + (d >= 0.05f ? "+" : "") + juce::String (d, 1) : juce::String ("OFF");
        g.drawText (v, 86, 0, getWidth() - 94, getHeight(), juce::Justification::centredRight);
    }
private:
    HadesProcessor& proc;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att;
};

//==============================================================================
class Canvas : public juce::Component
{
public:
    explicit Canvas (HadesProcessor& p)
        : proc (p), eq (p), presetBox (p), lu (p), sourceChips (p.apvts, "source", true),
          lowProtChip (p.apvts, "lowProt", "LOW PROTECT"), lowProtHz (p.apvts, "lowProtFreq", ""),
          transChip (p.apvts, "transKeep", "TRANSIENT KEEP"), routingChips (p.apvts, "routing", true),
          osChips (p.apvts, "oversample", true), inDrag (p.apvts, "inGain", "IN "), tiltDrag (p.apvts, "tilt", "TILT "),
          outDrag (p.apvts, "outGain", "OUT "), flux (p.apvts)
    {
        addAndMakeVisible (eq);
        addAndMakeVisible (presetBox);
        addAndMakeVisible (lu);
        addAndMakeVisible (sourceChips);
        for (int i = 0; i < 2; ++i)
        {
            auto* b = ab.add (new juce::TextButton (i == 0 ? "A" : "B"));
            b->getProperties().set ("bold", true);
            b->onClick = [this, i] { proc.selectAB (i); updateAB(); };
            addAndMakeVisible (b);
        }
        updateAB();

        ageSlider.setSliderStyle (juce::Slider::LinearHorizontal);
        ageSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        ageAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "age", ageSlider);
        ageSlider.setDoubleClickReturnValue (true, 1.0);
        ageSlider.onValueChange = [this] { repaint (ageArea); };
        addAndMakeVisible (ageSlider);

        for (int i = 0; i < 6; ++i)
        {
            auto* c = cards.add (new ModuleCard (p, i, [this] (int k) { select (k); }));
            addAndMakeVisible (c);
        }

        using K = ModuleDrawer::K; using T = ModuleDrawer::T;
        drawers.add (new ModuleDrawer (p.apvts, "HEAT", "heatMode",
            { K { "heatTone", "TONE", true }, K { "heatLo", "FOCUS LO" }, K { "heatHi", "FOCUS HI" }, K { "heatMix", "MIX" } }, {}));
        drawers.add (new ModuleDrawer (p.apvts, "TAPE", "tapeSpeed",
            { K { "tapeBump", "BUMP" }, K { "tapeWear", "WEAR" }, K { "tapeDrops", "DROPS" }, K { "tapeDrift", "VOL DRIFT" },
              K { "tapeMix", "MIX" }, K { "dustLevel", "DUST" } }, {}, "dustType", "DUST"));
        drawers.add (new ModuleDrawer (p.apvts, "DRIFT", "driftMode",
            { K { "driftRate", "RATE" }, K { "driftSpread", "SPREAD" }, K { "driftMix", "MIX" } }, {}));
        drawers.add (new ModuleDrawer (p.apvts, "GLUE", "glueMode",
            { K { "glueAttack", "ATTACK" }, K { "glueRelease", "RELEASE" }, K { "glueScHpf", "SC HPF" }, K { "glueMix", "MIX" } },
            { T { "glueAuto", "AUTO REL" } }));
        drawers.add (new ModuleDrawer (p.apvts, "GRAIN", "grainTime",
            { K { "grainFree", "FREE TIME" }, K { "grainSize", "SIZE" }, K { "grainDensity", "DENSITY" }, K { "grainSpray", "SPRAY" },
              K { "grainPitch", "PITCH", true }, K { "grainReverse", "REVERSE" }, K { "grainFeedback", "FEEDBK" }, K { "grainDuck", "DUCK" },
              K { "grainLo", "FOCUS LO" }, K { "grainHi", "FOCUS HI" } },
            { T { "grainFreeze", "FREEZE" } }));
        drawers.add (new ModuleDrawer (p.apvts, "SPACE", "spaceMode",
            { K { "spacePre", "PRE" }, K { "spaceSize", "SIZE" }, K { "spaceDecay", "DECAY" }, K { "spaceDamp", "DAMP" }, K { "spaceMod", "MOD" },
              K { "spaceWidth", "WIDTH" }, K { "spaceDuck", "DUCK" }, K { "spaceLo", "FOCUS LO" }, K { "spaceHi", "FOCUS HI" } }, {}));
        for (auto* d : drawers) addChildComponent (d);
        addAndMakeVisible (flux);

        for (auto* c : std::initializer_list<juce::Component*> { &lowProtChip, &lowProtHz, &transChip, &routingChips, &osChips, &inDrag, &tiltDrag, &outDrag, &randomBtn })
            addAndMakeVisible (c);
        randomBtn.onClick = [this] { proc.randomize(); };
        routingP = p.apvts.getParameter ("routing");

        select (1);
    }

    void select (int k)
    {
        selected = k;
        for (int i = 0; i < cards.size(); ++i) cards[i]->setSelected (i == k);
        for (int i = 0; i < drawers.size(); ++i) drawers[i]->setVisible (i == k);
        flux.setModule (k);
    }
    void updateAB() { for (int i = 0; i < ab.size(); ++i) ab[i]->setToggleState (proc.getABSlot() == i, juce::dontSendNotification); }

    void tick (float dt)
    {
        for (auto* c : cards) c->tick (dt);
        eq.tick();
        flux.tick (dt);
        presetBox.refresh();
        lu.repaint();
        const float target[2] = { proc.inLevel.load(), proc.outLevel.load() };
        for (int i = 0; i < 2; ++i) meter[i] = std::max (target[i], meter[i] * 0.85f);
        repaint (meterArea);
        const int rt = juce::roundToInt (paramReal (routingP));
        if (rt != lastRouting) { lastRouting = rt; repaint (0, 220, 1200, 24); }
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (col::bg);
        g.setColour (col::ink); g.setFont (disp (34.f));
        g.drawText ("HADES", 24, 18, 180, 50, juce::Justification::centredLeft);
        const float hw = textWidth (disp (34.f), "HADES");
        g.setColour (col::muted); g.setFont (mono (11.f));
        g.drawText ("BY GERMO", 24 + (int) hw + 8, 38, 70, 20, juce::Justification::centredLeft);

        // AGE
        g.setColour (col::ink); g.setFont (mono (11.f, true));
        g.drawText ("AGE", ageArea.withHeight (16), juce::Justification::centredLeft);
        g.setColour (col::muted); g.setFont (mono (11.f));
        g.drawText (juce::String (juce::roundToInt (ageSlider.getValue() * 100)) + "%", ageArea.withHeight (16), juce::Justification::centredRight);

        // meters
        g.setFont (mono (9.f));
        for (int i = 0; i < 2; ++i)
        {
            auto m = meterArea.withWidth (22).translated (i * 26, 0).toFloat();
            const float h = juce::jlimit (0.f, 1.f, (gainToDb (meter[i]) + 60.f) / 60.f) * 34.f;
            g.setColour (col::track); g.fillRoundedRectangle (m.getX() + 3, m.getY(), 14, 34, 2.f);
            g.setColour (col::teal); g.fillRoundedRectangle (m.getX() + 3, m.getY() + 34 - h, 14, h, 2.f);
            g.setColour (col::muted);
            g.drawText (i == 0 ? "IN" : "OUT", (int) m.getX() - 4, (int) m.getY() + 36, 30, 12, juce::Justification::centred);
        }

        // chain label
        g.setColour (col::muted); g.setFont (mono (11.f));
        g.drawText ("CHAIN" + juce::String (juce::CharPointer_UTF8 (" \xc2\xb7 ")) + "CLICK A MODULE TO EDIT", 24, 222, 400, 18, juce::Justification::centredLeft);
        const juce::String arrow = juce::CharPointer_UTF8 (" \xe2\x86\x92 ");
        const juce::String chain = lastRouting == 1 ? "HEAT" + arrow + "TAPE" + arrow + "DRIFT" + arrow + "GLUE" + arrow + "[ GRAIN | SPACE ]"
                                                    : "HEAT" + arrow + "TAPE" + arrow + "DRIFT" + arrow + "GLUE" + arrow + "GRAIN" + arrow + "SPACE";
        g.drawText (chain, 600, 222, 576, 18, juce::Justification::centredRight);

        // drawer
        auto dr = drawerArea.toFloat().reduced (0.75f);
        g.setColour (col::panel); g.fillRoundedRectangle (dr, 12.f);
        g.setColour (col::ink); g.drawRoundedRectangle (dr, 12.f, 1.5f);
        g.setColour (col::ktrack);
        g.drawLine ((float) flux.getX() - 20, dr.getY() + 18, (float) flux.getX() - 20, dr.getBottom() - 18, 1.f);

        // footer labels
        g.setColour (col::muted); g.setFont (mono (10.f));
        g.drawText ("ROUTING", routingChips.getX() - 62, 814, 58, 28, juce::Justification::centredRight);
        g.drawText ("OS", osChips.getX() - 26, 814, 22, 28, juce::Justification::centredRight);
    }

    void resized() override
    {
        presetBox.setBounds (266, 22, 250, 46);
        ab[0]->setBounds (528, 29, 36, 32);
        ab[1]->setBounds (568, 29, 36, 32);
        lu.setBounds (616, 29, 142, 32);
        const int sw = sourceChips.preferredWidth();
        sourceChips.setBounds (770, 28, sw, 36);
        ageArea = { 770 + sw + 14, 24, 1124 - (770 + sw + 14) - 8, 44 };
        ageSlider.setBounds (ageArea.getX() - 2, ageArea.getY() + 18, ageArea.getWidth() + 4, 24);
        meterArea = { 1124, 22, 52, 50 };

        eq.setBounds (24, 84, 1152, 128);

        const int cw = (1152 - 5 * 12) / 6;
        for (int i = 0; i < 6; ++i) cards[i]->setBounds (24 + i * (cw + 12), 246, cw, 286);

        drawerArea = { 24, 546, 1152, 254 };
        for (auto* d : drawers) d->setBounds (drawerArea.getX() + 18, drawerArea.getY() + 16, 1152 - 18 - 300, 230);
        flux.setBounds (drawerArea.getRight() - 268, drawerArea.getY() + 16, 250, 200);

        int x = 24;
        lowProtChip.setBounds (x, 814, 120, 30); x += 124;
        lowProtHz.setBounds (x, 814, 72, 30); x += 84;
        transChip.setBounds (x, 814, 140, 30); x += 212;
        routingChips.setBounds (x, 812, routingChips.preferredWidth(), 34); x += routingChips.preferredWidth() + 40;
        osChips.setBounds (x, 812, osChips.preferredWidth(), 34);
        int rx = 1176;
        randomBtn.setBounds (rx - 108, 814, 108, 30); rx -= 116;
        outDrag.setBounds (rx - 112, 814, 112, 30); rx -= 120;
        tiltDrag.setBounds (rx - 112, 814, 112, 30); rx -= 120;
        inDrag.setBounds (rx - 104, 814, 104, 30);
    }

private:
    HadesProcessor& proc;
    EqPanel eq;
    PresetBox presetBox;
    LuButton lu;
    ChoiceChips sourceChips;
    juce::OwnedArray<juce::TextButton> ab;
    juce::Slider ageSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ageAtt;
    juce::OwnedArray<ModuleCard> cards;
    juce::OwnedArray<ModuleDrawer> drawers;
    ToggleChip lowProtChip;
    DragValue lowProtHz;
    ToggleChip transChip;
    ChoiceChips routingChips, osChips;
    DragValue inDrag, tiltDrag, outDrag;
    juce::TextButton randomBtn { "RANDOMIZE" };
    FluxPanel flux;
    juce::RangedAudioParameter* routingP = nullptr;
    juce::Rectangle<int> ageArea, meterArea, drawerArea;
    float meter[2] {};
    int selected = 1, lastRouting = -1;
};

//==============================================================================
HadesEditor::HadesEditor (HadesProcessor& p) : juce::AudioProcessorEditor (&p)
{
    setLookAndFeel (&lnf);
    canvas = std::make_unique<Canvas> (p);
    addAndMakeVisible (*canvas);
    setResizable (true, true);
    setResizeLimits (720, 516, 1560, 1118);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio (1200.0 / 860.0);
    setSize (1020, 731);
    startTimerHz (30);
}

HadesEditor::~HadesEditor()
{
    stopTimer();
    canvas.reset();
    setLookAndFeel (nullptr);
}

void HadesEditor::paint (juce::Graphics& g) { g.fillAll (col::bg); }

void HadesEditor::resized()
{
    const float s = (float) getWidth() / 1200.f;
    canvas->setBounds (0, 0, 1200, 860);
    canvas->setTransform (juce::AffineTransform::scale (s));
}

void HadesEditor::timerCallback() { canvas->tick (1.f / 30.f); }
} // namespace hd
