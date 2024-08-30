from PySide2 import QtCore, QtWidgets
import maya.cmds as cmds
import maya.OpenMayaUI as OpenMayaUI1
from maya.app.general.mayaMixin import MayaQWidgetDockableMixin
from shiboken2 import wrapInstance
import os
import json
import unittest
import math
class JSONAnimExport:
    def __init__(self, folder_path):
        self.folderPath = folder_path
    def sanitize_name(self, name):
        specialChars = "!#$%^&*():"
        for specialChar in specialChars:
            name = name.replace(specialChar, '_')
        return name

    def get_name_space(self,target):
        if ":" in target:
            namespace = target.split(":")[0]
            self.namespace = namespace
        else:
            self.namespace = 'default'
    def get_cameras(self):
        all_cameras = cmds.ls(type='camera', l=True)
        cameras = []
        default_cameras = ["top", "front", "side", "persp"]
        for cam in all_cameras:
            camera_transform = cmds.listRelatives(cam, p=True)[0]
            if camera_transform not in default_cameras:
                cameras.append(camera_transform)
        if cameras:
            return cameras
        else:
            return []
    def get_lastkey(self,cameras):
        max_frame = -1
        for camera in cameras:
            key_frames = cmds.keyframe(camera, query=True,lsl = True)
            if key_frames:
                max_frame = max(max_frame, max(key_frames))
            else:
                max_frame = 0
                print(f"No keyframes found for {camera}")
        if max_frame > -1:
            return int(math.ceil(max_frame))
        else:
            return max_frame  
    def get_frames_to_JSON(self):
        self.cameras = self.get_cameras()
        if not self.cameras:
            print("No cameras found.")
            return  

        self.get_name_space(self.cameras[0])
        self.lastkey = self.get_lastkey(self.cameras)
        self.saveDirectory = os.path.join(self.folderPath, self.namespace)
        if not os.path.exists(self.saveDirectory):
            os.makedirs(self.saveDirectory)

        for f in range(self.lastkey + 1):
            cmds.currentTime(f)
            filePath = os.path.join(self.saveDirectory, f"frame_{f}.json")
            frame_data = {}

            for camera in self.cameras:
                if self.namespace and ':' not in camera:
                    right_name = f'{self.namespace}:{camera}'
                else:
                    right_name = camera
                try:
                    tx = cmds.getAttr(f"{camera}.tx")
                    ty = cmds.getAttr(f"{camera}.ty")
                    tz = cmds.getAttr(f"{camera}.tz")
                    rx = cmds.getAttr(f"{camera}.rx")
                    ry = cmds.getAttr(f"{camera}.ry")
                    rz = cmds.getAttr(f"{camera}.rz")
                    frame_data[right_name] = {
                        "tx": tx, "ty": ty, "tz": tz,
                        "rx": rx, "ry": ry, "rz": rz,
                    }
                except Exception as e:
                    print(f"Error accessing attributes of {right_name}: {str(e)}")
            
            with open(filePath, 'w') as file:
                json.dump(frame_data, file, indent=4)
# this case is idle:Hips        
class TestJSONExport(unittest.TestCase):
    def setUp(self):
        self.exporter = JSONAnimExport()
    def test_get_camera(self):
        cameras = self.exporter.get_cameras()
        expected_result = ['camera1', 'camera2']
        self.assertEqual(cameras,expected_result)
    def test_namespace(self):
        cameras = self.exporter.get_cameras()
        self.exporter.get_name_space(cameras[0])
        self.assertEqual(self.exporter.namespace,'default')
    def test_lastkey(self):
        cameras = self.exporter.get_cameras()
        maxFrame = self.exporter.get_lastkey(cameras)
        self.assertEqual(maxFrame,34)
    def test_frame_JSON(self):
        self.exporter.get_frames_to_JSON()
        count = 0
        # Iterate directory
        for path in os.listdir(self.exporter.saveDirectory):
            # check if current path is a file
            if os.path.isfile(os.path.join(self.exporter.saveDirectory, path)):
                count += 1
                # frame count:

        self.assertEqual(count,self.exporter.lastkey+1)
            
def suite():
    suite = unittest.TestSuite()
    suite.addTest(TestJSONExport('test_get_camera'))
    suite.addTest(TestJSONExport('test_namespace'))
    suite.addTest(TestJSONExport('test_lastkey'))
    suite.addTest(TestJSONExport('test_frame_JSON'))
    return suite
def run_tests():
    runner = unittest.TextTestRunner()
    test_suite = suite()
    runner.run(test_suite)
    
#run_tests()  

def get_main_window():
    window = OpenMayaUI1.MQtUtil.mainWindow()
    return wrapInstance(int(window), QtWidgets.QWidget)

class CameraExportUI(MayaQWidgetDockableMixin, QtWidgets.QDialog):
    def __init__(self, parent=get_main_window()):
        super(CameraExportUI, self).__init__(parent)
        self.setWindowTitle('Camera Animation Export')
        self.resize(300, 200)
        self.folder_dir = QtWidgets.QLineEdit()
        self.folder_button = QtWidgets.QPushButton('Browse...')
        self.export_button = QtWidgets.QPushButton('Export Camera Data')
        self.progress_bar = QtWidgets.QProgressBar(maximum=100)
        self.status_label = QtWidgets.QLabel('Ready')
        self.create_layout()
        self.create_connections()
    
    def create_layout(self):
        main_layout = QtWidgets.QVBoxLayout(self)
        folder_layout = QtWidgets.QHBoxLayout()
        folder_layout.addWidget(self.folder_dir)
        folder_layout.addWidget(self.folder_button)
        main_layout.addLayout(folder_layout)
        main_layout.addWidget(self.export_button)
        main_layout.addWidget(self.progress_bar)
        main_layout.addWidget(self.status_label)

    def create_connections(self):
        self.folder_button.clicked.connect(self.select_folder)
        self.export_button.clicked.connect(self.export_camera_data)

    def select_folder(self):
        folder = QtWidgets.QFileDialog.getExistingDirectory(self, "Select Folder")
        if folder:
            self.folder_dir.setText(folder)

    def export_camera_data(self):
        folder = self.folder_dir.text()
        if not folder:
            QtWidgets.QMessageBox.warning(self, "Error", "Please specify a folder.")
            return
        exporter = JSONAnimExport(folder)
        exporter.get_frames_to_JSON()
        self.progress_bar.setValue(100)
        self.status_label.setText('Export Completed Successfully')



if __name__ == "__main__":
    try:
        camera_dialog.close()
        camera_dialog.deleteLater()
    except:
        pass

    camera_dialog = CameraExportUI()
    camera_dialog.show()
