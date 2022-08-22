import time
import subprocess
import os
from pathlib import Path
import glob


def get_parent_cwd():
    return Path(__file__).parent.parent.resolve()


def add_dataset_from_path_generate(path, all_domains, all_problems):
    cwd = get_parent_cwd()
    domains = glob.glob(os.path.join(cwd, path)+"domain*")
    problems = glob.glob(os.path.join(cwd, path)+"p*")
    domains.sort()
    problems.sort()
    all_domains += domains
    all_problems += problems


def generate_files():
    cwd = get_parent_cwd()
    all_domains = []
    all_problems = []
    path_openstacks = "domains/openstacks/"
    path_pathways = "domains/pathways/"
    path_TPP = "domains/TPP/"
    add_dataset_from_path_generate(path_openstacks, all_domains, all_problems)
    add_dataset_from_path_generate(path_pathways, all_domains, all_problems)
    add_dataset_from_path_generate(path_TPP, all_domains, all_problems)
    os.chdir(os.path.join(cwd, "transpiler_costs"))
    command_generator = os.path.join(os.getcwd(), "generator.py")
    for n_rules in range(0, MAX_ETHICAL_RULES+1, int(MAX_ETHICAL_RULES/MAX_CASES)):
        for (domain, problem) in zip(all_domains, all_problems):
            os.system('python -B '+command_generator+' ' +
                      domain+' '+problem+' '+str(n_rules))
        print('Generated files with rules: '+str(n_rules))


def add_dataset_from_path(path, dataset, key):
    cwd = get_parent_cwd()
    dataset[key] = {}
    for i in range(0, MAX_ETHICAL_RULES+1, int(MAX_ETHICAL_RULES/MAX_CASES)):
        domains = glob.glob(os.path.join(cwd, path)+str(i)+"/domain*")
        problems = glob.glob(os.path.join(cwd, path)+str(i)+"/p*")
        domains.sort()
        problems.sort()
        dataset[key][i] = zip(domains, problems)


def run_tests(run_number):
    cwd = get_parent_cwd()
    dataset = {}
    # Dataset: Dict<String, Dict<Integer, Pair<Domain, Problem>>>
    path_openstacks = "domains/openstacks/"
    path_pathways = "domains/pathways/"
    path_TPP = "domains/TPP/"
    add_dataset_from_path(path_openstacks, dataset, 'openstacks')
    add_dataset_from_path(path_pathways, dataset, 'pathways')
    add_dataset_from_path(path_TPP, dataset, 'TPP')
    # Command
    os.chdir(os.path.join(cwd, "lprpgp"))
    command_planner = os.path.join(os.getcwd(), "plan")

    for test_case in dataset.keys():
        print(test_case)
        with open(test_case+"."+str(run_number)+".txt", "w+") as file:
            file.write('rules,time \n')
            print('rules,time')
            for n_rules in dataset[test_case].keys():
                run_time = 0
                n_probs = 0
                for (domain, problem) in dataset[test_case][n_rules]:
                    n_probs += 1
                    FNULL = open(os.devnull, 'w')
                    start = time.time()
                    print(command_planner)
                    print(domain)
                    print(problem)
                    subprocess.call([command_planner, domain, problem,
                                    'out/out.txt'], stdout=FNULL, stderr=subprocess.STDOUT)
                    run_time += time.time() - start
                run_time /= (n_probs)
                file.write(str(n_rules)+','+str(run_time)+' \n')
                print(str(n_rules)+','+str(run_time))


# Main procedure
MAX_ETHICAL_RULES = 22
MAX_CASES = 11
RUNS = 1

for i in range(0, RUNS):
    # generate_files()
    run_tests(i)
