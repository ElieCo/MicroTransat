from SimuManager import Manager
from SimuModelTester import ModelTester
import sys

if "--log" in sys.argv:
    filename = sys.argv[-1]

    tester = ModelTester()
    tester.testSimulator(filename)

else:
    manager = Manager()
    manager.runSimulation()
