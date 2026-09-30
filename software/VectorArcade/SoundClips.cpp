#include "SoundClips.h"

#if SOUNDS_FROM_LITTLEFS()
#include <LittleFS.h>
#include <Streaming.h>
#include "sounds/SoundsId.h"

// ----------------------------------------------------------------------------------------
bool loadSoundClip(SoundEngine& sound, FileClip& clip, const char* path)
{
  unsigned long t0 = millis();

  if (sound.load(clip, LittleFS, path) == false)
  {
    Serial << "LittleFS: " << path << " could not be loaded" << endl;
    return false;
  }

  Serial << "LittleFS: " << path << " loaded, " << clip.getSize() << " bytes in "
         << (millis() - t0) << " ms" << endl;
  return true;
}

// ----------------------------------------------------------------------------------------
bool checkSoundFiles()
{
  File f = LittleFS.open("/sounds.id", "r");
  if (!f)
  {
    Serial << "LittleFS: /sounds.id missing - upload the contents of data/" << endl;
    return false;
  }

  String id = f.readString();
  f.close();
  id.trim();

  if (id != cSoundsId)
  {
    Serial << "LittleFS: sound files do not match this firmware (found " << id << ", expected "
           << cSoundsId << ") - upload the contents of data/" << endl;
    return false;
  }

  Serial << "LittleFS: sound files match (id " << id << ")" << endl;
  return true;
}
#endif
