#!/usr/bin/python

# This is skull workflow config controlling script

import sys
import os
import getopt
import string
import pprint
import yaml

yaml_obj = None
config_name = ""

def _load_yaml_config(config_name):
    yaml_file = file(config_name, 'r')
    yml_obj = yaml.load(yaml_file)
    return yml_obj

def _create_workflow():
    return {
    'concurrent' : 1,
    'idl': "",
    'stdin': 0,
    'port' : -1,
    'modules' : []
    }

################################# Workflow Actions #############################
def process_workflow_actions():
    try:
        opts, args = getopt.getopt(sys.argv[5:7], 'a:')

        action = ""
        for op, value in opts:
            if op == "-a":
                action = value

        if action == "add":
            _process_add_workflow()
        elif action == "show":
            _process_show_workflow()
        elif action == "gen_idl":
            _process_gen_workflow_idl()
        elif action == "value":
            _process_show_workflow_item()
        elif action == "count":
            _process_workflow_count()

    except Exception as e:
        print "Fatal: process_add_workflow: " + str(e)
        raise

def _process_workflow_count():
    global yaml_obj
    global config_name

    workflows = yaml_obj['workflows']
    return workflows and 0 or len(workflows)

def _process_show_workflow_item():
    global yaml_obj
    global config_name

    try:
        opts, args = getopt.getopt(sys.argv[7:], 'i:d:')
        workflow_idx = 0

        for op, value in opts:
            if op == "-i":
                workflow_idx = int(value)
            elif op == "-d":
                workflow_field = value

        workflow_frame = yaml_obj['workflows'][workflow_idx]
        print "{}".format(workflow_frame[workflow_field])

    except Exception as e:
        print "Fatal: _process_show_workflow_item: " + str(e)
        raise

def _process_show_workflow():
    workflows = yaml_obj['workflows']
    workflow_cnt = 0

    if workflows is None:
        print "no workflow"
        print "note: run 'skull workflow --add' to create a new workflow"
        return

    for workflow in workflows:
        print "workflow [%d]:" % workflow_cnt
        print " - concurrent: %s" % workflow['concurrent']
        print " - idl: %s" % workflow['idl']

        if workflow.get("port"):
            print " - port: %s" % workflow['port']

        if workflow.get("stdin"):
            print " - stdin: %d" % workflow['stdin']

        print " - modules:"
        print "   -",

        modules = workflow['modules']
        numOfModule = len(modules)

        for i in range(numOfModule):
            module = modules[i]

            if i == 0:
                print ("%s" % module),
            else:
                print ("-> %s" % module),

        # increase the workflow count
        workflow_cnt += 1
        print "\n\n",

    print "total %d workflows" % (workflow_cnt)

def _process_add_workflow():
    global yaml_obj
    global config_name

    try:
        opts, args = getopt.getopt(sys.argv[7:], 'C:i:p:I:')

        workflow_concurrent = 1
        workflow_idl = ""
        workflow_port = -1
        workflow_stdin = 0

        for op, value in opts:
            if op == "-C":
                workflow_concurrent = int(value)
            elif op == "-i":
                workflow_idl = value
            elif op == "-p":
                workflow_port = int(value)
            elif op == "-I":
                workflow_stdin = int(value)

        # 1. Now add these workflow_x to yaml obj and dump it
        workflow_frame = _create_workflow()
        workflow_frame['concurrent'] = workflow_concurrent
        workflow_frame['idl'] = workflow_idl

        # 1.1 Add trigger
        if workflow_stdin == 1:
            workflow_frame['stdin'] = 1
        elif workflow_port > 0:
            workflow_frame['port'] = workflow_port

        # 2. create if the workflows list do not exist
        if yaml_obj['workflows'] is None:
            yaml_obj['workflows'] = []

        # 3. update the skull-config.yaml
        yaml_obj['workflows'].append(workflow_frame)
        yaml.dump(yaml_obj, file(config_name, 'w'))

    except Exception as e:
        print "Fatal: _process_add_workflow: " + str(e)
        raise

def _process_gen_workflow_idl():
    workflows = yaml_obj['workflows']

    if workflows is None:
        print "no workflow"
        print "note: run 'skull workflow --add' to create a new workflow"
        return

    idl_name = ""
    idl_path = ""

    opts, args = getopt.getopt(sys.argv[7:], 'n:p:')
    for op, value in opts:
        if op == "-n":
            idl_name = value
        elif op == "-p":
            idl_path = value

    for workflow in workflows:
        workflow_idl_name = workflow['idl']

        if idl_name != workflow_idl_name:
            continue

        proto_file_name = "%s/%s.proto" % (idl_path, idl_name)
        proto_file = file(proto_file_name, 'w')
        content = "// This is generated by framework\n"
        content += "// Notes:\n"
        content += "//  - The top message is the main message structure we use,\n"
        content += "//     if you want to define some sub-messages, write then\n"
        content += "//     after top message\n"
        content += "//  - Do not recommend to use 'required' field, if you really\n"
        content += "//     want to use it, make sure to fill it before use it\n"
        content += "\n"
        content += "package skull.workflow;\n\n"
        content += "message %s {\n" % idl_name
        content += "    optional bytes data = 1;\n"
        content += "}\n"

        proto_file.write(content)
        proto_file.close()
        break

################################# Module Actions ###############################
def process_module_actions():
    try:
        opts, args = getopt.getopt(sys.argv[5:7], 'a:')

        action = ""
        for op, value in opts:
            if op == "-a":
                action = value

        if action == "add":
            _process_add_module()

    except Exception as e:
        print "Fatal: process_module_actions: " + str(e)
        raise

def _module_valid(module_list, module_name, module_type):
    if module_list is None:
        return True

    for item in module_list:
        items = str(item).split(":")
        name = str(items[0])
        type = items[1]

        if name == module_name and type != module_type:
            return False

    return True

def _process_add_module():
    global yaml_obj
    global config_name

    try:
        opts, args = getopt.getopt(sys.argv[7:], 'M:i:l:')

        workflow_idx = 0
        module_name = ""
        type = ""

        for op, value in opts:
            if op == "-M":
                module_name = value
            elif op == "-i":
                workflow_idx = int(value)
            elif op == "-l":
                type = value

        # Now add these workflow_x to yaml obj and dump it
        workflow_frame = yaml_obj['workflows'][workflow_idx]

        # Check whether the module has exist in module list
        module_list = workflow_frame['modules']
        if _module_valid(module_list, module_name, type):
            workflow_frame['modules'].append(module_name + ":" + type)
        else:
            print "Error: there is already a module with different type"
            raise

        yaml.dump(yaml_obj, file(config_name, 'w'))

        print "add module done"

    except Exception as e:
        print "Fatal: _process_add_module: " + str(e)
        raise

################################## Service Actions #############################
def process_service_actions():
    try:
        opts, args = getopt.getopt(sys.argv[5:7], 'a:')

        action = ""
        service_name = ""

        for op, value in opts:
            if op == "-a":
                action = value

        if action == "exist":
            _process_service_exist()
        elif action == "add":
            _process_add_service()
        elif action == "show":
            _process_show_service()
        else:
            print "Fatal: Unknown service action: %s" % action
            raise

    except Exception as e:
        print "Fatal: process_service_actions: " + str(e)
        raise

def _process_show_service():
    services = yaml_obj['services']
    services_cnt = 0

    if services is None:
        print "no service"
        print "note: run 'skull service --add' to create a new service"
        return

    for name in services:
        service_item = services[name]

        print "service [%s]:" % name
        print " - type: %s" % service_item['type']
        print " - enable: %s" % service_item['enable']
        print " - data_mode: %s" % service_item['data_mode']

        # increase the service count
        services_cnt += 1
        print "\n",

    print "total %d services" % (services_cnt)

def _process_service_exist():
    services = yaml_obj['services']

    if services is None:
        sys.exit(1)

    service_name = ""
    opts, args = getopt.getopt(sys.argv[7:], 's:')

    for op, value in opts:
        if op == "-s":
            servie_name = value

    service = services.get(service_name)
    if service is None:
        sys.exit(1)
    else:
        sys.exit(0)

def _process_add_service():
    global yaml_obj
    global config_name

    try:
        opts, args = getopt.getopt(sys.argv[7:], 's:b:d:l:')
        service_name = ""
        service_enable = True
        service_data_mode = ""
        service_yml_config = ""
        service_language = ""

        for op, value in opts:
            if op == "-s":
                service_name = value
            elif op == "-b":
                service_enable = value
            elif op == "-d":
                service_data_mode = value
            elif op == "-l":
                service_language = value

        # If service dict do not exist, create it
        if yaml_obj['services'] is None:
            yaml_obj['services'] = {}

        # Now add a service item into services dict
        services = yaml_obj['services']
        service_obj = {
            'type'     : service_language,
            'enable'   : service_enable,
            'data_mode': service_data_mode
        }

        services[service_name] = service_obj;

        yaml.dump(yaml_obj, file(config_name, 'w'))

    except Exception as e:
        print "Fatal: _process_add_service: " + str(e)
        raise

################################################################################
def usage():
    print "usage:"
    print "  skull-config-utils.py -m workflow -c $yaml_file -a show"
    print "  skull-config-utils.py -m workflow -c $yaml_file -a add -C $concurrent -i $idl_name -p $port"
    print "  skull-config-utils.py -m workflow -c $yaml_file -a gen_idl -n $idl_name -p $idl_path"

    print "  skull-config-utils.py -m module   -c $yaml_file -a add -M $module_name -i $workflow_index"

    print "  skull-config-utils.py -m service  -c $yaml_file -a show"
    print "  skull-config-utils.py -m service  -c $yaml_file -a exist -s $service_name"
    print "  skull-config-utils.py -m service  -c $yaml_file -a add -s $service_name -b $enabled -d $data_mode -l language"

if __name__ == "__main__":
    if len(sys.argv) == 1:
        usage()
        sys.exit(1)

    try:
        action = None
        opts, args = getopt.getopt(sys.argv[1:5], 'c:m:')

        for op, value in opts:
            if op == "-c":
                config_name = value
                yaml_obj = _load_yaml_config(config_name)
            elif op == "-m":
                action = value

        # Now run the process func according the mode
        if action == "workflow":
            process_workflow_actions()
        elif action == "module":
            process_module_actions()
        elif action == "service":
            process_service_actions()
        else:
            print "Fatal: Unknown action: %s" % action

    except Exception as e:
        print "Fatal: " + str(e)
        usage()
        raise
