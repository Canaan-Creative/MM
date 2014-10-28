#!/bin/bash

# Automated Continuous Integration script
#   Check revisions from git://github.com/BitSyncom/mm.git, build if changed

set -e

BASEDIR=$(readlink -e $(dirname $0))
. $BASEDIR/aci.conf

REPO=$WORKDIR/repo
REVISION_LOG=$WORKDIR/revision.log
REVISION_NEW=$REVISION_LOG.new
BUILD_DIR=`date +%Y%m%d_%H%M`
DIRECTORY=mm
URL=git://github.com/BitSyncom/mm.git

CORE_NUM="$(expr $(nproc) + 1)"
[ -z "$CORE_NUM" ] && CORE_NUM=2

# We prefer curl because of wget bugs
which wget > /dev/null && DL_PROG=wget && DL_PARA="-nv -O"
which curl > /dev/null && DL_PROG=curl && DL_PARA="-L -o"

[ ! -d $REPO ] && mkdir $REPO
echo -ne > $REVISION_NEW

cd $WORKDIR
cd $REPO

if [ -d $REPO/$DIRECTORY ]; then
        cd $REPO/$DIRECTORY
        git pull || { cd $REPO; rm -rf $REPO/$DIRECTORY; }
fi
if [ ! -d $REPO/$DIRECTORY ]; then
        cd $REPO
        git clone $URL ./$DIRECTORY || { echo "Clone git failed: $URL"; exit 1; }
        cd $DIRECTORY
        git checkout -b $GITBRANCH origin/$GITBRANCH || { echo "Branch git failed: $URL"; exit 1; }
        cd ..
fi
cd $REPO/$DIRECTORY
REVISION=`git log | head -n 1 | cut -d " " -f2`
[ -z "$REVISION" ] && { echo "Get git revision failed: $URL"; exit 1; }

echo "$REVISION" > $REVISION_NEW

# Revision compare
[ ! -d $WORKDIR/$BUILD_DIR ] && mkdir -p $WORKDIR/$BUILD_DIR && chmod 0700 $WORKDIR/$BUILD_DIR
BUILD_LOG=$WORKDIR/$BUILD_DIR/$BUILD_DIR.log
cd $WORKDIR
if diff $REVISION_NEW $REVISION_LOG 2>&1 > $BUILD_LOG; then
        rm $REVISION_NEW
        rm -rf $WORKDIR/$BUILD_DIR
        echo "Revision has not been changed"
        exit 0
else
        my_mail "Avalon (mm) Build Start: $BUILD_DIR" "`echo --DIFF-- && echo && diff $REVISION_NEW $REVISION_LOG && echo` `echo --NEW-- && echo && cat $REVISION_NEW` `echo --OLD-- && echo && cat $REVISION_LOG`"

        echo "---------------- OLD REVISION ----------------"   >> $BUILD_LOG
        [ -r $REVISION_LOG ] && cat $REVISION_LOG               >> $BUILD_LOG
        echo                                                    >> $BUILD_LOG
        echo "---------------- NEW REVISION ----------------"   >> $BUILD_LOG
        cat $REVISION_NEW                                       >> $BUILD_LOG
        echo                                                    >> $BUILD_LOG

        cd $WORKDIR/$BUILD_DIR
        git clone $URL
        cd $DIRECTORY
        git checkout -b $GITBRANCH origin/$GITBRANCH

        # /home/Xilinx/14.6/ISE_DS/ --> $(HOME)/Xilinx/14.7/ISE_DS/
        # firmware/Makefile and synth/xilinx.mk
        sed -i 's:/home/Xilinx/14.6/ISE_DS/:$(HOME)/opt/Xilinx/14.7/ISE_DS/:g' firmware/Makefile
        sed -i 's:/home/Xilinx/14.6/ISE_DS/:$(HOME)/opt/Xilinx/14.7/ISE_DS/:g' synth/xilinx.mk

        # /opt/lm32 --> [$WORKDIR]/lm32
        # firmware/config.mk and firmware/toolchain/Makefile
        sed -i "s:/opt/lm32:$WORKDIR/$BUILD_DIR/lm32:g" firmware/config.mk
        sed -i "s:/opt/lm32:$WORKDIR/$BUILD_DIR/lm32:g" firmware/toolchain/Makefile

        TIME_BEGIN=`date +"%Y%m%d %H:%M:%S"`
        BUILD_BEGIN=`date +%s`
        (make -j${CORE_NUM} -C firmware/toolchain || make -C firmware/toolchain)        >> $BUILD_LOG 2>&1
        (make -j${CORE_NUM} -C firmware || make -C firmware)                            >> $BUILD_LOG 2>&1
        RET="$?"
        TIME_END=`date +"%Y%m%d %H:%M:%S"`
        BUILD_END=`date +%s`
        BUILD_COST=0
        BUILD_H=0
        BUILD_M=0
        BUILD_S=0
        [ "${BUILD_END}" -gt "${BUILD_BEGIN}" ] && BUILD_COST=`expr ${BUILD_END} - ${BUILD_BEGIN}`
        expr ${BUILD_COST} / 3600 > /dev/null 2>&1 && BUILD_H=`expr ${BUILD_COST} / 3600`
        expr ${BUILD_COST} / 60 % 60 > /dev/null 2>&1 && BUILD_M=`expr ${BUILD_COST} / 60 % 60`
        expr ${BUILD_COST} % 60 > /dev/null 2>&1 && BUILD_S=`expr ${BUILD_COST} % 60`

        echo                                                                    >> $BUILD_LOG
        echo "############################################################"     >> $BUILD_LOG
        echo " FROM  ${TIME_BEGIN}  TO  ${TIME_END}"                            >> $BUILD_LOG
        echo " BUILD RETURN : ${RET}"                                           >> $BUILD_LOG
        echo " TIME COST ${BUILD_H}:${BUILD_M}:${BUILD_S}"                      >> $BUILD_LOG
        echo "############################################################"     >> $BUILD_LOG
        echo                                                                    >> $BUILD_LOG

        cd $WORKDIR
        my_mail "Avalon (mm) Build End ${BUILD_DIR}$([ $RET != 0 ] && echo -FAILED-${RET})" \
                "`echo ============================================================ && echo` \
                 `echo  FROM  ${TIME_BEGIN}  TO  ${TIME_END} && echo` \
                 `echo  BUILD RETURN : ${RET} && echo` \
                 `echo  TIME COST ${BUILD_H}:${BUILD_M}:${BUILD_S} && echo` \
                 `echo ============================================================ && echo` \
                 `echo && echo && echo --DIFF-- && echo && diff $REVISION_NEW $REVISION_LOG` \
                 `echo && echo && echo --NEW-- && echo && cat $REVISION_NEW && echo && echo --OLD-- && echo && cat $REVISION_LOG` \
                 `echo && echo && echo --ls-- && ls -lR $WORKDIR/$BUILD_DIR/$DIRECTORY/firmware/bin/`"
        if [ "$RET" == "0" ]; then
                cp -rpd $WORKDIR/$BUILD_DIR/$DIRECTORY/firmware/bin/* $WORKDIR/$BUILD_DIR/
                rm -rf $WORKDIR/$BUILD_DIR/$DIRECTORY $WORKDIR/$BUILD_DIR/lm32
                chmod 0755 $WORKDIR/$BUILD_DIR
                mv $REVISION_NEW $REVISION_LOG
        else
                mv $WORKDIR/$BUILD_DIR $WORKDIR/"${BUILD_DIR}"_failed
                touch $WORKDIR/.fail
                exit 1
        fi
        exit 0
fi
