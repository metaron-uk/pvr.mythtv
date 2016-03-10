DIR=$(dirname $0)
KODI_PVR=$DIR/pvr

BASE_URL=https://github.com/xbmc/xbmc/trunk
OPTIONS="--force --trust-server-cert --non-interactive"
svn export $OPTIONS $BASE_URL/xbmc/cores/VideoPlayer/DVDDemuxers/DVDDemuxPacket.h $KODI_PVR/DVDDemuxPacket.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/libXBMC_pvr.h $KODI_PVR/libXBMC_pvr.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/libXBMC_codec.h $KODI_PVR/libXBMC_codec.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/libKODI_guilib.h $KODI_PVR/libKODI_guilib.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/libXBMC_addon.h $KODI_PVR/libXBMC_addon.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/dlfcn-win32.h $KODI_PVR/dlfcn-win32.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/dlfcn-win32.cpp $KODI_PVR/dlfcn-win32.cpp
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_addon_cpp_dll.h $KODI_PVR/xbmc_addon_cpp_dll.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_addon_dll.h $KODI_PVR/xbmc_addon_dll.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_addon_types.h $KODI_PVR/xbmc_addon_types.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_codec_types.h $KODI_PVR/xbmc_codec_types.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_epg_types.h $KODI_PVR/xbmc_epg_types.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_pvr_dll.h $KODI_PVR/xbmc_pvr_dll.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_pvr_types.h $KODI_PVR/xbmc_pvr_types.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_scr_dll.h $KODI_PVR/xbmc_scr_dll.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_scr_types.h $KODI_PVR/xbmc_scr_types.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_vis_dll.h $KODI_PVR/xbmc_vis_dll.h
svn export $OPTIONS $BASE_URL/xbmc/addons/kodi-addon-dev-kit/include/kodi/xbmc_vis_types.h $KODI_PVR/xbmc_vis_types.h
svn export $OPTIONS $BASE_URL/xbmc/filesystem/IFileTypes.h $KODI_PVR/IFileTypes.h
