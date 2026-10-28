// SPDX-License-Identifier: AGPL-3.0-only
#include "SoundPresets.h"
#include "ParameterLimits.h"
#include <cmath>

namespace yuro
{
static bool valid(int i, float value)
{
    if (!std::isfinite(value)) return false;
    if (i==2 || i==3) return value>=.005f && value<=maxEnvelopeSeconds;
    if (i>=8 && i<=10) return value>=-maxEqDb && value<=maxEqDb;
    if (i>=11) return value>=0 && value<=3 && std::floor(value)==value;
    return value>=0 && value<=1;
}
bool CustomPresetStore::save(const SoundPreset& sound) const
{
    for(int i=0;i<13;++i)if(!valid(i,sound.values[static_cast<size_t>(i)]))return false;
    if(!file.getParentDirectory().createDirectory())return false;
    juce::DynamicObject::Ptr object(new juce::DynamicObject());
    object->setProperty("format", "YURODSTVO_CUSTOM");
    object->setProperty("version", 1);
    juce::Array<juce::var> values;
    for(const auto value:sound.values)values.add(static_cast<double>(value));
    object->setProperty("values",values);
    juce::TemporaryFile temporary(file);
    if(!temporary.getFile().replaceWithText(juce::JSON::toString(juce::var(object.get()),true)))return false;
    return temporary.overwriteTargetFileWithTemporary();
}
bool CustomPresetStore::load(SoundPreset& sound) const
{
    if(!file.existsAsFile() || file.getSize()>16384)return false;
    const auto parsed=juce::JSON::parse(file);
    auto* object=parsed.getDynamicObject();
    if(!object || object->getProperty("format").toString()!="YURODSTVO_CUSTOM"
        || static_cast<int>(object->getProperty("version"))!=1)return false;
    const auto array=object->getProperty("values").getArray();
    if(!array || array->size()!=13)return false;
    SoundPreset candidate;
    for(int i=0;i<13;++i)
    {
        if(!(*array)[i].isDouble() && !(*array)[i].isInt())return false;
        const auto value=static_cast<float>(static_cast<double>((*array)[i]));
        if(!valid(i,value))return false;
        candidate.values[static_cast<size_t>(i)]=value;
    }
    sound=candidate;
    return true;
}
}
