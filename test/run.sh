EXEDIR="../x64/Release"
RESOURCEDIR="../Resources"
RESULTDIR="./"
SCALE=4
MARGIN=1

${EXEDIR}/OneBitTiff.exe ${RESOURCEDIR}/bfr-1bit-c.tif ${RESULTDIR}/bfr-8bit-c.tif ${SCALE} ${MARGIN}
${EXEDIR}/OneBitTiff.exe ${RESOURCEDIR}/bfr-1bit-m.tif ${RESULTDIR}/bfr-8bit-m.tif ${SCALE} ${MARGIN}
${EXEDIR}/OneBitTiff.exe ${RESOURCEDIR}/bfr-1bit-y.tif ${RESULTDIR}/bfr-8bit-y.tif ${SCALE} ${MARGIN}
${EXEDIR}/OneBitTiff.exe ${RESOURCEDIR}/bfr-1bit-k.tif ${RESULTDIR}/bfr-8bit-k.tif ${SCALE} ${MARGIN}

${EXEDIR}/Compose.exe ${RESULTDIR}/bfr-8bit-c.tif ${RESULTDIR}/bfr-8bit-m.tif ${RESULTDIR}/bfr-8bit-y.tif ${RESULTDIR}/bfr-8bit-k.tif ${RESULTDIR}/bfr-8bit-4cmyk.tif



