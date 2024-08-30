import maya.api.OpenMaya as om
import maya.cmds as cmds
import maya._OpenMayaUI as OpenMayaUI1
from maya.app.general.mayaMixin import MayaQWidgetDockableMixin
from animExporter import CameraExportUI

'''
def maya_useNewAPI():
    """
    Can either use this function (which works on earlier versions)
    or we can set maya_useNewAPI = True
    """
    pass
'''

maya_useNewAPI = True
camera_dialog = None
def CameraExportUIScript(restore=False):
    global camera_dialog
    if restore == True:
        restored_control = OpenMayaUI1.MQtUtil.getCurrent()
    if camera_dialog is None:
        print("creating new ui")
        camera_dialog =CameraExportUI()
    if restore == True :
        mixin_ptr = OpenMayaUI1.MQtUtil.findControl(camera_dialog.objectName())
        OpenMayaUI1.MQtUtil.addWidgetToMayaLayout(int(mixin_ptr),int(restored_control))
    else:
        camera_dialog.show(dockable=True,width=300,height=200,
                                uiScript='CameraExportUIScript(restore=True)')
    

class animExporter(om.MPxCommand):

    CMD_NAME = "animExporter"

    def __init__(self):
        super(animExporter, self).__init__()

    def doIt(self, args):
        ui=CameraExportUIScript()
        if ui is not None:
            try: 
                cmds.workspaceControl('JSONAnimExportWorkspaceControl', e=True, restore = True)
            except:
                pass
        return ui
        


    @classmethod
    def creator(cls):
        """
        Think of this as a factory
        """
        return animExporter()


def initializePlugin(plugin):
    """
    Load our plugin
    """
    vendor = "DingJia"
    version = "1.0.0"

    plugin_fn = om.MFnPlugin(plugin, vendor, version)

    try:
        plugin_fn.registerCommand(animExporter.CMD_NAME, animExporter.creator)
    except:
        om.MGlobal.displayError(
            "Failed to register command: {0}".format(animExporter.CMD_NAME)
        )


def uninitializePlugin(plugin):
    """
    Exit point for a plugin
    """
    plugin_fn = om.MFnPlugin(plugin)
    try:
        plugin_fn.deregisterCommand(animExporter.CMD_NAME)
    except:
        om.MGlobal.displayError(
            "Failed to deregister command: {0}".format(animExporter.CMD_NAME)
        )


if __name__ == "__main__":
    """
    So if we execute this in the script editor it will be a __main__ so we can put testing code etc here
    Loading the plugin will not run this
    As we are loading the plugin it needs to be in the plugin path.
    """

    plugin_name = "animExporter.py"

    cmds.evalDeferred(
        'if cmds.pluginInfo("{0}", q=True, loaded=True): cmds.unloadPlugin("{0}")'.format(
            plugin_name
        )
    )
    cmds.evalDeferred(
        'if not cmds.pluginInfo("{0}", q=True, loaded=True): cmds.loadPlugin("{0}")'.format(
            plugin_name
        )
    )
