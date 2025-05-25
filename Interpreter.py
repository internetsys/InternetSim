import re
from enum import Enum, auto

from Topology import Topology
from AS import AS
from Extension import Extension

class Policy(Enum):
    AS_PEER = auto()
    FILTER_IN = auto()
    FILTER_OUT = auto()
    ADD_RULE = auto()
    REMOVE_RULE = auto()
    SHOW_RULE = auto()

peers = set()

class PolicyBlock:
    __slots__ = ("value", "subBlock")

    def __init__(self, value):
        self.value = value
        self.subBlock = []

    def init(self):
        peer = PolicyBlock([("AS (\d+) peer AS (\d+)", Policy.AS_PEER)])

        self.subBlock.append(peer)

        filter = PolicyBlock(
            [
                ("filter in", Policy.FILTER_IN),
                ("filter out", Policy.FILTER_OUT),
            ]
        )

        peer.subBlock.append(filter)

        rule = PolicyBlock(
            [
                ("add rule", Policy.ADD_RULE),
                ("remove rule (\d+)", Policy.REMOVE_RULE),
                ("show rule", Policy.SHOW_RULE),
            ]
        )

        filter.subBlock.append(rule)

    def parse(self, content, topology, args=None):
        global peers

        while True:

            try:
                line = next(content).strip()

            except StopIteration:
                if self.value == "root":
                    return
                
                else:
                    raise SyntaxError("Unexpected end of policy")

            if line == "end":
                return

            flag = False  # check if the command is valid

            for subBlock in self.subBlock:
                for item in subBlock.value:

                    result = re.match(item[0], line)

                    if result:

                        flag = True

                        if item[1] == Policy.AS_PEER:

                            ASNx = int(result.group(1))
                            ASNy = int(result.group(2))

                            if ASNx not in topology.ASes:
                                raise ValueError(f"Invalid AS number: {ASNx}")

                            if ASNy not in topology.ASes:
                                raise ValueError(f"Invalid AS number: {ASNy}")

                            subBlock.parse(
                                content,
                                topology=topology,
                                args=(ASNx, ASNy),
                            )

                        elif item[1] == Policy.FILTER_IN:

                            # args[0] = ASNx, args[1] = ASNy (peer AS)
                            peers.add((args[0], args[1]))

                            subBlock.parse(
                                content,
                                topology=topology,
                                args=(args[0], args[1], "filter in"),
                            )

                        elif item[1] == Policy.FILTER_OUT:

                            # args[0] = ASNx, args[1] = ASNy (peer AS)
                            peers.add((args[1], args[0]))

                            subBlock.parse(
                                content,
                                topology=topology,
                                args=(args[0], args[1], "filter out"),
                            )

                        elif item[1] == Policy.ADD_RULE:

                            while True:
                                line = next(content).strip()

                                if line == "end":
                                    break

                                line = re.sub(r'"\s*([^"]*?)\s*"', r'"\1"', line)

                                result = re.match('match\s+"([^"]*)"', line)

                                if not result:
                                    raise SyntaxError(f"Invalid match: {line}")

                                match = result.group(1)
                                if not (
                                    match == "any"
                                    or re.match("prefix in (.+)", match)
                                    or re.match("prefix is (.+)", match)
                                    or re.match("community contains (\d+:\d+)", match)
                                    or re.match("path '(.+)'", match)
                                    or re.match("AS-SET (.+)", match)
                                ):
                                    raise SyntaxError(f"Invalid match: {line}")

                                line = next(content).strip()
                                line = re.sub(r'"\s*([^"]*?)\s*"', r'"\1"', line)

                                result = re.match('\s*action\s+"([^"]*)"', line)
                                if not result:
                                    raise SyntaxError(f"Invalid action: {line}")

                                action = result.group(1)
                                if (
                                    action == "accept"
                                    or action == "deny"
                                    or re.match("local-pref (\d+)", action)
                                    or re.match("community add (\d+:\d+)", action)
                                    or re.match("community strip", action)
                                    or re.match("community remove (\d+:\d+)", action)
                                    or re.match("prepend (\d+)", action)
                                ):
                                    # args[0] = ASNx, args[1] = ASNy (peer AS), args[2] = IN/OUT (filter type)
                                    topology.ASes[args[0]].addFilter(
                                        args[1], args[2], match, action
                                    )

                                    topology.policyNum += 1

                                else:

                                    raise SyntaxError(f"Invalid action: {line}")

                        elif item[1] == Policy.REMOVE_RULE:

                            ruleNum = result.group(1)

                            # args[0] = ASNx, args[1] = ASNy (peer AS), args[2] = IN/OUT (filter type)
                            if not topology.ASes[args[0]].removeFilter(
                                args[1], args[2], ruleNum
                            ):
                                raise ValueError(f"Invalid rule number: {ruleNum}")
                            
                            topology.policyNum -= 1

                        elif item[1] == Policy.SHOW_RULE:

                            # args[0] = ASNx, args[1] = ASNy (peer AS), args[2] = IN/OUT (filter type)
                            filter = topology.ASes[args[0]].getFilter(args[1], args[2])

                            if filter == []:
                                print(
                                    f"AS {args[0]} peer AS {args[1]} {args[2]} no rules"
                                )

                            else:

                                for rule in enumerate(filter):
                                    print(
                                        f'AS {args[0]} peer AS {args[1]} {args[2]} rule {rule[0]}: match "{rule[1][0]}", action "{rule[1][1]}"'
                                    )

            if not flag:
                raise SyntaxError(f"Invalid command in Routing Policy: {line}")

'''
from prompt_toolkit import prompt
from prompt_toolkit.history import InMemoryHistory

from Engine import CommandType

history = InMemoryHistory()

class CommandLine:
    __slots__ = ("value", "subCommands")

    def __init__(self, value):
        self.value = value
        self.subCommands = []

    def addSubCommand(self, subCommand):
        self.subCommands.append(subCommand)

    def init(self):
        ASCommand = CommandLine(
            [
                ("AS (\d+) peer AS (\d+)", CommandType.AS_PEER),
                ("AS (\d+) show peers", CommandType.SHOW_PEERS),
                ("AS (\d+) show routes (.+)", CommandType.SHOW_ROUTES),
            ]
        )

        self.addSubCommand(ASCommand)

        filterCommand = CommandLine(
            [
                ("filter in", CommandType.FILTER_IN),
                ("filter out", CommandType.FILTER_OUT),
            ]
        )

        refreshRouteCommand = CommandLine(
            [
                ("refresh route", CommandType.REFRESH_ROUTE),
            ]
        )

        ASCommand.addSubCommand(filterCommand)
        ASCommand.addSubCommand(refreshRouteCommand)

        ruleCommand = CommandLine(
            [
                ("add rule", CommandType.ADD_RULE),
                ("remove rule (\d+)", CommandType.REMOVE_RULE),
                ("show rule", CommandType.SHOW_RULE),
            ]
        )

        filterCommand.addSubCommand(ruleCommand)

    def parse(self, content, topology=None, scheduler=None, args=None):
        while True:
            try:
                global history

                line = prompt(
                    content + ">", history=history, enable_history_search=True
                ).strip()

            except EOFError:
                exit()

            if line == "end":
                return

            flag = False

            for subCommand in self.subCommands:
                for item in subCommand.value:

                    result = re.match(item[0], line)

                    if result:

                        flag = True

                        if item[1] == CommandType.AS_PEER:

                            ASNx = int(result.group(1))
                            ASNy = int(result.group(2))

                            if ASNx not in topology.ASes or ASNy not in topology.ASes:
                                print("Error: Invalid AS number")
                                break

                            hasPeer = False
                            for peer in topology.ASes[ASNx].getPeers():
                                if peer[0] == ASNy:
                                    hasPeer = True
                                    break

                            if not hasPeer:
                                print("Error: No such peer AS")
                                break

                            subCommand.parse(
                                line,
                                topology=topology,
                                scheduler=scheduler,
                                args=(ASNx, ASNy),
                            )

                        elif item[1] == CommandType.SHOW_PEERS:

                            ASN = int(result.group(1))

                            if ASN not in topology.ASes:
                                print("Error: Invalid AS number")
                                break

                            peers = topology.ASes[ASN].getPeers()
                            ASNlist = []

                            for peer in peers:
                                ASNlist.append(peer[0])

                        elif item[1] == CommandType.SHOW_ROUTES:

                            ASN = int(result.group(1))
                            prefix = result.group(2)

                            if ASN not in topology.ASes:
                                print("Error: Invalid AS number")
                                break

                            topology.ASes[ASN].showRoute(prefix)

                        elif item[1] == CommandType.FILTER_IN:

                            # args[0] = ASNx, args[1] = ASNy (peer AS)
                            subCommand.parse(
                                line, topology=topology, args=(args[0], args[1], "IN")
                            )

                        elif item[1] == CommandType.FILTER_OUT:

                            # args[0] = ASNx, args[1] = ASNy (peer AS)
                            subCommand.parse(
                                line, topology=topology, args=(args[0], args[1], "OUT")
                            )

                        elif item[1] == CommandType.ADD_RULE:
                            while True:

                                exitFlag = False

                                while True:
                                    try:
                                        match = prompt(
                                            "add rule>match: ",
                                            history=history,
                                            enable_history_search=True,
                                        ).strip()
                                    except EOFError:
                                        exit()

                                    if match == "end":
                                        exitFlag = True
                                        break

                                    if (
                                        match == "any"
                                        or re.match("prefix in (.+)", match)
                                        or re.match("prefix is (.+)", match)
                                        or re.match(
                                            "community contains (\d+:\d+)", match
                                        )
                                        or re.match("path '(.+)'", match)
                                    ):
                                        break

                                    else:
                                        print("Error: Invalid match")

                                if exitFlag:
                                    break

                                while True:
                                    try:
                                        action = prompt(
                                            "add rule>action: ",
                                            history=history,
                                            enable_history_search=True,
                                        ).strip()
                                    except EOFError:
                                        exit()

                                    if action == "end":
                                        exitFlag = True
                                        break

                                    if (
                                        action == "accept"
                                        or action == "deny"
                                        or re.match("local-pref (\d+)", action)
                                        or re.match("community add (\d+:\d+)", action)
                                        or re.match("community strip", action)
                                        or re.match(
                                            "community remove (\d+:\d+)", action
                                        )
                                        or re.match("prepend (\d+)", action)
                                    ):
                                        # args[0] = ASNx, args[1] = ASNy (peer AS), args[2] = IN/OUT (filter type)
                                        topology.ASes[args[0]].addFilter(
                                            args[1], args[2], match, action
                                        )
                                        break

                                    else:
                                        print("Error: Invalid action")

                                if exitFlag:
                                    break

                                print("Info: Add success")

                        elif item[1] == CommandType.REMOVE_RULE:

                            # args[0] = ASNx, args[1] = ASNy (peer AS), args[2] = IN/OUT (filter type)
                            if topology.ASes[args[0]].removeFilter(
                                args[1], args[2], result.group(1)
                            ):
                                print("Info: Remove success")

                            else:
                                print("Error: Invalid rule number")

                        elif item[1] == CommandType.SHOW_RULE:

                            # args[0] = ASNx, args[1] = ASNy (peer AS), args[2] = IN/OUT (filter type)
                            filter = topology.ASes[args[0]].getFilter(args[1], args[2])

                            if filter == []:
                                print("Info: No rules")

                            else:
                                for rule in enumerate(filter):
                                    print(
                                        f'Rule {rule[0]}: Match "{rule[1][0]}", Action "{rule[1][1]}"'
                                    )

            if not flag:
                print("Error: Invalid command")

    @staticmethod
    def parseCmd():
        rootCommand = CommandLine("InternetSim")
        rootCommand.init()
        
        while True:
            rootCommand.parse("InternetSim")
            break
'''

class Interpreter:
    __slots__ = "policyBlock"

    def __init__(self):
        self.policyBlock = PolicyBlock("root")
        self.policyBlock.init()

    def loadRoutingInformation(self, config, topology):

        ASRelFilePath = config["Path"]["ASRelFilePath"]
        policyFilePath = config["Path"].get("policyFilePath")
        ASSetFilePath = config["Path"].get("ASSetFilePath")

        ROVSetFilePath = config["Path"].get("ROVSetFilePath")
        invalidSetFilePath = config["Path"].get("invalidSetFilePath")

        try:
            with open(ASRelFilePath, "r") as f:

                lineNum = 0

                for line in f:

                    lineNum += 1
                    line=line.strip()

                    if line.startswith("#") or not line:
                        continue
                    
                    try:
                        parts=line.split("|")

                        if len(parts) != 3:
                             raise ValueError(f"ValueError: Invalid format in ASRelFile at line {lineNum}: '{line}'")
                        
                        ASNxStr, ASNyStr, relationshipStr = parts
                        ASNx = int(ASNxStr)
                        ASNy = int(ASNyStr)
                        relationship = int(relationshipStr)

                        topology.addAS(int(ASNx), int(ASNy), int(relationship))

                    except ValueError as e:

                         raise ValueError(f"ValueError: Data conversion error in ASRelFile at line {lineNum}: {line} ({e})")
                    
                    except Exception as e:
                         
                         raise RuntimeError(f"RuntimeError: Error parsing ASRelFile at line {lineNum}: {line} ({e})")

                print(f"Loaded {topology.ASNum} ASes, {topology.linkNum} links")
        
        except FileNotFoundError:
            raise FileNotFoundError(f"ASRelFile not found: {ASRelFilePath}")
        
        except Exception as e:
            raise RuntimeError(f"Error reading ASRelFile: {ASRelFilePath} ({e})")

        if policyFilePath:

            try:
                with open(policyFilePath, "r") as f:
                    content = iter(f.readlines())
                    self.policyBlock.parse(content, topology)

                print(f"Loaded {topology.policyNum} rules")
            
            except FileNotFoundError:
                raise FileNotFoundError(f"policyFile not found: {policyFilePath}")
            
            except Exception as e:
                raise RuntimeError(f"Error reading policyFile: {ASRelFilePath} ({e})")
            


        if ASSetFilePath:
            try:
                AS.loadASSet(ASSetFilePath)
            
            except FileNotFoundError:
                raise FileNotFoundError(f"ASSetFile not found: {ASSetFilePath}")
            
            except Exception as e:
                raise RuntimeError(f"Error reading ASSetFile: {ASSetFilePath} ({e})")

        if ROVSetFilePath:
            try:
                Extension.loadROVSet(ROVSetFilePath)

            except FileNotFoundError:
                raise FileNotFoundError(f"ROVSetFile not found: {ROVSetFilePath}")
            
            except Exception as e:
                raise RuntimeError(f"Error reading ROVSetFile: {ROVSetFilePath} ({e})")

        if invalidSetFilePath:
            try:
                Extension.loadInvalidSet(invalidSetFilePath)
            
            except FileNotFoundError:
                raise FileNotFoundError(f"invalidSetFile not found: {invalidSetFilePath}")
            
            except Exception as e:
                raise RuntimeError(f"Error reading invalidSetFile: {invalidSetFilePath} ({e})")

    def changeRoutingPolicy(self, topology, policy):
        content = iter(policy.splitlines())
        global peers
        peers.clear()

        self.policyBlock.parse(content, topology)

        return peers

    def changeNonBGPPolicy(self, topology, policy):
        ASes = set()

        for line in policy.splitlines():
            line = line.strip()

            if line.startswith("#") or not line:
                continue

            result = re.match("AS (\d+) enable ROV", line)

            if result:
                ASNStr= result.group(1)

                try:
                    ASN = int(ASNStr)
                    Extension.enableROV(ASN)

                    ASes.add(ASN)

                    continue
                
                except ValueError:
                    raise ValueError(f"Invalid AS number {ASNStr} in Non-BGP Policy: {line}")
                
                except Exception as e:
                    raise RuntimeError(f"Error {e} in Non-BGP Policy: {line}")

            elif line == "end":
                return ASes
            
            else:
                raise SyntaxError(f"Invalid command in Non-BGP Policy: {line}")
