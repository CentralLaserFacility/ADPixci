IF NOT DEFINED EPICS_HOST_ARCH (
    SET EPICS_HOST_ARCH=windows-x64-static
)
..\..\bin\%EPICS_HOST_ARCH%\pixci st.cmd
