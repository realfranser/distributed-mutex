import filecmp
from os import pipe
import subprocess

test_lib = {
    0: "CUATROPROCESOS.ord",
    1: "UNOMISMO.ord"
}

bash_command = "./controlador Ejemplos/DATSI/"

count = 0

for test in test_lib.values():
    this_test = bash_command + test
    print(this_test)
    process = subprocess.Popen(this_test.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()

    with open('Ejemplos/DATSI/result_'+(str)(count), 'r') as file:
        data = file.read().replace('\n', '')

    print((str)(output).replace('\n', ' '))
    print(data)
    print(output == data)

    count += 1
