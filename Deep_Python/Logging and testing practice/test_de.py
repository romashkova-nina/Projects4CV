import pytest
import numpy as np
import random
from differential_evolution import DifferentialEvolution


def rastrigin(array, A=10):
    return A * 2 + (array[0] ** 2 - A * np.cos(2 * np.pi * array[0])) + (
            array[1] ** 2 - A * np.cos(2 * np.pi * array[1]))


BOUNDS = np.array([[-20, 20], [-20, 20]])
FOBJ = rastrigin

def test_to_check_parameters_DE():
    # Этот тест проверяет, что модель была верно иницилизирована и все параметры встали на свои места.

    SEED = 35
    random.seed(SEED)
    np.random.seed(SEED)

    de_solver = DifferentialEvolution(FOBJ, BOUNDS, mutation_coefficient=0.3, crossover_coefficient=0.5,
                                      population_size=20)
    de_solver._init_population()

    population_sample = np.array(
        [[0.45805495, 0.30834961], [0.23148705, 0.27742455], [0.81723481, 0.11134664], [0.62643723, 0.27678789],
         [0.68217467, 0.67897078], [0.79671742, 0.04580216], [0.91259827, 0.21381599], [0.3036373, 0.98906362],
         [0.1858815, 0.98872484], [0.75008423, 0.22238605], [0.14790391, 0.51579028], [0.39425832, 0.06988013],
         [0.33822577, 0.01103722], [0.76752786, 0.87472213], [0.53359432, 0.08441275], [0.8243312, 0.5045812],
         [0.88161863, 0.17404628], [0.40295789, 0.83212654], [0.97866247, 0.61916477], [0.86992066, 0.2488769]])

    population_sample_denorm = np.array(
        [[-1.67780207, -7.66601572], [-10.7405182, -8.90301793], [12.68939248, -15.54613456], [5.05748916, -8.92848453],
         [7.28698697, 7.15883136], [11.86869674, -18.1679135], [16.50393098, -11.44736055], [-7.85450795, 19.56254494],
         [-12.56473988, 19.54899342], [10.00336925, -11.1045582], [-14.08384367, 0.631611], [-4.22966726, -17.20479499],
         [-6.47096919, -19.55851115], [10.70111432, 14.98888522], [1.34377283, -16.62349002], [12.97324783, 0.1832478],
         [15.26474508, -13.03814861], [-3.88168455, 13.28506176], [19.14649887, 4.76659061], [14.79682621, -10.044924]])

    fitness_sample = np.array([91.00053494, 207.01770031, 436.00249576, 106.93403313, 121.2320226, 479.22149127,
                               442.87676285, 467.51956153, 578.74977552, 225.46231626, 216.88074163, 329.81870527,
                               463.57469632, 352.22840999, 310.84087599, 174.40740058, 414.21679216, 206.38382925,
                               402.21388769, 327.34238276])

    assert de_solver.fobj([2, 5]) == 29.0
    assert de_solver.bounds[0] == pytest.approx(np.array([-20, 20]))
    assert de_solver.mutation_coefficient == 0.3
    assert de_solver.crossover_coefficient == 0.5
    assert de_solver.population_size == 20
    assert de_solver.dimensions == len(de_solver.bounds)

    assert len(de_solver.population) == 20
    assert de_solver.population == pytest.approx(population_sample)
    assert de_solver.min_bound == pytest.approx(np.array([-20, -20]))
    assert de_solver.max_bound == pytest.approx(np.array([20, 20]))
    assert de_solver.diff == pytest.approx(np.array([40, 40]))
    assert de_solver.population_denorm == pytest.approx(population_sample_denorm)
    assert de_solver.fitness == pytest.approx(fitness_sample)
    assert de_solver.best_idx == 0
    assert de_solver.best == pytest.approx(np.array([-1.67780207, -7.66601572]))


def test_to_check_solution_DE_1():
    # Этот тест проверяет, что алгоритм верно отрабатывает и заполняет все сопряженные поля верными значениями (1-ый набор параметров).

    SEED = 27
    random.seed(SEED)
    np.random.seed(SEED)

    de_solver = DifferentialEvolution(FOBJ, BOUNDS, mutation_coefficient=0.9, crossover_coefficient=0.1,
                                      population_size=10)
    de_solver._init_population()
    for _ in range(20):
        de_solver.iterate()

    assert de_solver.a == pytest.approx(np.array([0.50224766, 0.48130302]))
    assert de_solver.b == pytest.approx(np.array([0.5246507, 0.44840327]))
    assert de_solver.c == pytest.approx(np.array([0.53047621, 0.52432136]))
    assert de_solver.mutant == pytest.approx(np.array([0.4970047, 0.41297675]))
    assert de_solver.best_idx == 9
    assert de_solver.best == pytest.approx(np.array([0.0281175, 0.95616555]))
    assert (de_solver.cross_points == [False, True]).all
    assert de_solver.trial == pytest.approx(np.array([0.50070294, 0.41297675]))
    assert de_solver.trial_denorm == pytest.approx(np.array([0.0281175, -3.48093005]))


def test_to_check_solution_DE_2():
    # Этот тест проверяет, что алгоритм верно отрабатывает и заполняет все сопряженные поля верными значениями (2-й набор прараметров).

    SEED = 19
    random.seed(SEED)
    np.random.seed(SEED)

    de_solver = DifferentialEvolution(FOBJ, BOUNDS, mutation_coefficient=0.2, crossover_coefficient=0.8,
                                      population_size=17)
    de_solver._init_population()
    for _ in range(14):
        de_solver.iterate()

    assert de_solver.a == pytest.approx(np.array([0.55058784, 0.47686009]))
    assert de_solver.b == pytest.approx(np.array([0.5507549, 0.52837506]))
    assert de_solver.c == pytest.approx(np.array([0.54792239, 0.47307953]))
    assert de_solver.mutant == pytest.approx(np.array([0.55115435, 0.48791919]))
    assert de_solver.best_idx == 1
    assert de_solver.best == pytest.approx(np.array([1.98673431, 1.01333346]))
    assert (de_solver.cross_points == [True, True]).all
    assert de_solver.trial == pytest.approx(np.array([0.55115435, 0.48791919]))
    assert de_solver.trial_denorm == pytest.approx(np.array([2.04617384, -0.48323227]))

"""
Ваша задача добиться 100% покрытия тестами DifferentialEvolution
Различные этапы тестирования логики разделяйте на различные функции
Запуск команды тестирования:
pytest -s test_de.py --cov-report=json --cov
"""
