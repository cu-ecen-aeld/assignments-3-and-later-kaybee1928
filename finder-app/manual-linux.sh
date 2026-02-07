#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
REQUIRED_FS=("bin" "dev" "etc" "home" "lib" "lib64" "proc" "sbin" "sys" "tmp" "usr" "var" "usr/bin" "usr/lib" "usr/sbin" "var/log")

SYSROOT=$(${CROSS_COMPILE}gcc --print-sysroot)

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

cp "${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image" "${OUTDIR}/Image"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
  sudo rm  -rf "${OUTDIR}"/rootfs
fi

# TODO: Create necessary base directories


mkdir -p "${OUTDIR}/rootfs"
for dir in "${REQUIRED_FS[@]}"; do
    mkdir -p "${OUTDIR}/rootfs/${dir}"
done


cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} distclean
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX="${OUTDIR}"/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

cd "${OUTDIR}"/rootfs
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"



# TODO: Add library dependencies to rootfs

PROG_INTERP=$(${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter" | sed -n 's/.*: \(.*\)\]/\1/p')
echo "Program interpreter: ${PROG_INTERP}"

SHARED_LIBS=$(${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library" | awk -F'[][]' '{print $2}')
echo "Shared libraries:"
echo "${SHARED_LIBS}"

PROG_INTERP_DIR=$(dirname "${PROG_INTERP}")
mkdir -p "${OUTDIR}/rootfs${PROG_INTERP_DIR}"
cp -L "${SYSROOT}${PROG_INTERP}" "${OUTDIR}/rootfs${PROG_INTERP}"

for lib in ${SHARED_LIBS}; do
    cp -L "${SYSROOT}/lib64/${lib}" "${OUTDIR}/rootfs/lib64/"
done

# TODO: Make device nodes

sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

# TODO: Clean and build the writer utility
cd "${FINDER_APP_DIR}"
CROSS_COMPILE=${CROSS_COMPILE} make clean
CROSS_COMPILE=${CROSS_COMPILE} make

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -r "${FINDER_APP_DIR}"/. "${OUTDIR}/rootfs/home/"
#cp -r "${FINDER_APP_DIR}/writer" "${OUTDIR}/rootfs/home"
#cp -r "${FINDER_APP_DIR}/finder.sh" "${OUTDIR}/rootfs/home"
#cp -r "${FINDER_APP_DIR}/finder-test.sh" "${OUTDIR}/rootfs/home"
#cp -r "${FINDER_APP_DIR}/finder.sh" "${OUTDIR}/rootfs/home"
#cp -r "${FINDER_APP_DIR}/finder.sh" "${OUTDIR}/rootfs/home"
cp -r "${FINDER_APP_DIR}/../conf/username.txt" "${OUTDIR}/rootfs/home"
rm -rf "${OUTDIR}/rootfs/home/conf"
mkdir -p "${OUTDIR}/rootfs/home/conf"
cp -r "${FINDER_APP_DIR}/../conf/username.txt" "${OUTDIR}/rootfs/home/conf"
cp -r "${FINDER_APP_DIR}/../conf/assignment.txt" "${OUTDIR}/rootfs/home"
cp -r "${FINDER_APP_DIR}/../conf/assignment.txt" "${OUTDIR}/rootfs/home/conf"

# TODO: Chown the root directory
sudo chown root "${OUTDIR}/rootfs"

# TODO: Create initramfs.cpio.gz
cd "${OUTDIR}"/rootfs
find . | cpio -H newc -ov --owner root:root > "${OUTDIR}/initramfs.cpio"
cd "${OUTDIR}"
gzip -f initramfs.cpio

