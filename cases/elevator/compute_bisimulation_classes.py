import sys
import time
from pyeda.inter import *
import copy

class LTS:
#################################################################
#                                                               #
# INPUT has to be in the form of a .aut file with the following #
# characteristic:                                               #
#                                                               #
# 1) actions parameters of which the first is a single          #
#    feature or no parameter (which implicitly is               #
#    interpreted as the feature expression tt)                  #
#                                                               #
#################################################################

  def __init__(self, init = 0):
    self. init = init
    self. transitions = {}   # We store transitions as a function from S -> List(Action x FExpr x S)
    self. evaluation = {}
    self. predecessors = {}
    self. Stable_Classes = set( [] )
    self. Partition = {}

  def print_aut (self):
    for source in self. transitions:
      for (action, expression, destination) in self. transitions[source]:
        print('%d ---%s | %s----> %d' %(source, action, bdd2expr(expression), destination))

  def read_aut (self, name,classes = expr('1')):
    f = open (name,'r')
    description = f. readline (). lstrip (). rstrip ().split (',')
    initial_state = int(description[0][5:])
    transition_count = int(description[1].strip())
    state_count = int(description[2][:-1].strip())

    self. init = int (initial_state)
    for transition in f:
      (source, label, destination) = transition. split ('"')
      source = int (source[1:-1])
      destination = int (destination.rstrip()[1:-1])
      action = label
      expression = expr('1') # expr2bdd(classes)
      index = label. find('(')
      if index != -1:
        action = label[:index]
        if label[index+1:index+5] == 'node':
          argument_end = label. find(')')           # very bluntly assume there is only one feature on the edge; the remaining characters belong to the action
          action = action + label[argument_end+1:]
          (proposition, expr1, expr2) = label[index+6:argument_end]. split(',')
          if expr1 == 'tt': expression = expr2bdd(expr(proposition))
          else: expression = expr2bdd(expr('~'+proposition))
        elif label[index+1:index+3] == 'tt': expression = expr2bdd(expr('1')) # expr2bdd(classes)
        elif label[index+1:index+3] == 'ff': expression = expr2bdd(expr('0'))
      try: self. transitions[source]. append( (action,expression,destination) )
      except: self. transitions[source] =  [ (action,expression,destination) ]
      try: self. predecessors[destination] .append( (source,expression) )
      except: self. predecessors[destination] = [ (source,expression) ]
    f. close ()



  def Refine(self,classes = expr('1')):
    # classes is an expression encoding the Feature Diagram; if absent, we assume it is 'true'
    self. Partition = { s : set([expr2bdd(classes)]) for s in self. transitions }
    iteration = 0

    def Size(partition):
      return sum([len(partition[s]) for s in partition])

    def IsSatisfiable(FeatureExpression):
      return not(FeatureExpression.is_zero())

    def IsTautology(FeatureExpression):
      return FeatureExpression.is_one()

    # function for testing stability of classes; returns a non-stable class if not stable
    def Check_Stability (Family):
      R = { s : set([]) for s in self. Partition }
      R[0], Todo = { Family }, { (0, Family) }
      b = True                                                     # b holds iff (0,Family) is a stable partition
      unstable = (0, Family)                                       # in case (0,Family) is not stable, we return the closest splitter
      while Todo != set([]) and b:
        (s,e) = Todo. pop()
        for (action,feature,destination) in self. transitions[s]:
          if IsSatisfiable(e & feature):                           # transition is present for some product
            for family in self. Partition[destination]:
              if IsSatisfiable(e & feature & family):              # there is a product p in e & feature & family such that (s,e) --action|feature--> (destination, family)
                b = b and IsTautology( ~(e & feature) | family )   # if one product can reach this class, all should be able to reach this class
                if b and not(family in R[destination]):            # if this class can be reached and we have not yet seen it, add it to Todo
                  Todo |= { (destination, e & feature & family  ) }
                  R[destination] |= { family }
                if not(b): unstable = (s,e)
      if b:
        print('* [time %s] Detected stable family %s' %(time.strftime('%X %x %Z'),bdd2expr(Family)) )
        return True
      else: return (s,e)    
    # end Stability test

    # function for identifying reachable classes in self. Partition
    def Filter_Reachable ():
      #print ('* cleaning partition by removing unreachable classes')
      R = { s : set([]) for s in self. Partition }
      for Family in self. Partition[0]:
        Seen = { s : expr2bdd(expr('0')) for s in self. Partition }        # initialise Seen, which contains unions of families visited per state
        Todo = { (0, Family ) }
        Seen[0] = Family                                     # for the root, we know that we start with family Family
        while Todo != set([]):
          (s,e) = Todo. pop()
          for (action,feature,destination) in self. transitions[s]:
            if IsSatisfiable( e & feature ):                        # transition is present for some product from class (s,e)
              for family in self. Partition[destination]:
                if IsSatisfiable(e & feature & family):             # there is a product p in e & feature & family such that (s,e) --action|feature--> (destination, family)
                  if not(IsTautology(~(e & feature) | Seen[destination])): # there was this as well??? or destination == 0:
                    Todo |= { (destination, e & feature & family  ) }  # we have a product in (destination, family) that we did not see before
                  Seen[destination] = Seen[destination] | family    # we can reach (destination, family), so we add this class to Seen
                  R[destination] |= {family}                        # we mark family as reachable
      i,j = Size(self. Partition), Size(R)
      if j < i: print ('* [time %s] reachability analysis removed %d unreachable classes (new nr. of classes: %d)' %(time.strftime('%X %x %Z'), i-j, j ))
      self. Partition = R. copy ()                                                # since R contains all reachable classes, we can copy those to self. Partition
      return


    def Initialise_Partition ():         # refine with respect to the outgoing transitions
      print ('* [time %s] initialising partition' %(time.strftime('%X %x %Z') ))
      Todo = { (s, Family) for s in self. Partition for Family in self.Partition[s] }
      root_classes = self. Partition[0]
      while Todo != set([]):
        (s,Family) = Todo. pop ()  # pick a class
        for (action, feature, destination) in self. transitions[s]:
          if IsSatisfiable(Family & ~feature) and IsSatisfiable(Family & feature): # transition cannot be taken by all products in Family
            self. Partition[s] = (self. Partition[s] - { Family } ) | ( { Family & feature, Family & ~feature } )
            Todo |= { (s, Family & feature), (s, Family & ~feature) }

        if self.Partition[0] != root_classes:
          for Family in self. Partition[0]:
            stable = Check_Stability(Family)   # check whether P is stable
            if stable == True:                         # if we found a stable class, then we can remove all classes reachable from it
              self. Stable_Classes |= { Family }
              Filter_Reachable ()             
          self. Partition[0] = self. Partition[0] - self. Stable_Classes
      print ('* [time %s] finished initialising partition' %(time.strftime('%X %x %Z') ))
      return

    def Initialise_Partition_fast ():
      print ('* [time %s] initialising partition' %(time.strftime('%X %x %Z') ))
      for s in self. Partition:
        features = set([ feature for (action, feature, destination) in self. transitions[s] ]) - { expr2bdd(expr('1')) }
        for f in features:
          classes = self. Partition[s]
          for g in classes: 
            if f != g:
              self. Partition[s] = self. Partition[s] - { g }
              if IsSatisfiable(g & f): self. Partition[s] |= {g & f}
              if IsSatisfiable(g & ~f): self. Partition[s] |= {g & ~f}
      print ('* [time %s] finished initialising partition' %(time.strftime('%X %x %Z') ))
      return

    def Determine_Splitter (s, Family): # returns a splitter for (s,Family) if one exists, and False otherwise
      for (action,feature,destination) in self. transitions[s]:
        if IsSatisfiable(Family & feature):
          for Target_Family in self. Partition[destination]:
            if IsSatisfiable(Family & feature & Target_Family) and IsSatisfiable(Family & feature & ~Target_Family):
              return (feature, Target_Family)    # return splitter for s;
      return False                               # return that we did not find a splitter


    # actual refinement algorithm      
    #Initialise_Partition ()
    Initialise_Partition_fast ()
    root_classes = self. Partition[0]
    #print (' Analysing reachability               \r', end = '')
    #Filter_Reachable ()

    # initialise todo queue
    print (' Initialising todo queue              \r', end = '')
    U = [ (s, Family) for s in self. Partition for Family in self. Partition[s] if Determine_Splitter(s, Family) != False ]
    print ('\n Finished initialising queue at time %s. Queue size: %d classes. Refining partition...\n ' %(time.strftime('%X %x %Z'),len(U)))

    # main loop
    while U != []:
      (s, Family) = U[0] 
      U = U[1:]
      iteration += 1
      splitter = Determine_Splitter(s, Family)   # splitter != False iff (s, Family) has a splitter
      if splitter != False:                              
        (feature, Target_Family) = splitter # do the actual splitting: update the partition
        self. Partition[s] = (self. Partition[s] - {Family} ) | {Family & Target_Family, Family & ~Target_Family} 
        # add classes that may have become unstable to the queue
        #N = [ (t, f) for (t,feature) in self. predecessors[s] for f in self. Partition[t] if\
        #      IsSatisfiable(f & feature & Target_Family) or IsSatisfiable(f & feature & ~Target_Family)]    # or should be and?
        N = [ (t, f) for (t,feature) in self. predecessors[s] for f in self. Partition[t] if\
              IsSatisfiable(f & feature & Target_Family) and IsSatisfiable(f & feature & ~Target_Family)]
        for (t, f) in N: 
          if (t, f) in U: U. remove( (t,f) )  # remove occurrence of (pred, f) to prevent exact duplicates
        N. extend(U) 
        U = N. copy()  # we consider the predecessors of s first as new candidates for splitting

      if root_classes != self. Partition[0] or (iteration % 1250) == 0:  # check whether there are stable partitions or do a cleanup once in a while
        new_stable = False
        for e in self. Partition[0]: 
          stable = Check_Stability(e)
          if stable == True: 
            self. Stable_Classes |= { e }
          else: # stable = (s,e) for some pair (s,e)
            if stable in U: U. remove(stable)
            U = [stable]+U
            
          new_stable = stable or new_stable
        self. Partition[0] = self. Partition[0] - self. Stable_Classes
        #if new_stable or (iteration % 1000 == 0): 
        if (iteration % 1250 == 0): 
          Filter_Reachable ()
          U = [ (t, g) for (t, g) in U if Determine_Splitter(t, g) != False ]
        root_classes = self. Partition[0]

    # end of the loop
    # we print out all classes
    print(' The %d distinct families of products for this FTS are: ' %(len(self. Stable_Classes) ) )
    for Family in self. Stable_Classes:
      print ('* %s' %(bdd2expr(Family) ) )
  # the end of the refine method



# body of the program
     
if len(sys.argv) != 2:
  print('Expected exactly one argument: a .aut filename (including the .aut)')
  sys. exit()

name = sys.argv[1]

if name[-4:] == '.aut':
  print('Reading file %s at time %s' %(name,time.strftime('%X %x %Z')))
  sys.stdout.flush()
  g = LTS(0)
  # the Feature Diagram of the Minepump model
  g. read_aut (name)
  print('Finished reading FTS at time %s' %(time.strftime('%X %x %Z')))
  sys.stdout.flush()
  #g. print_aut ()
  g. Refine()
  print('Finished processing FTS at time %s' %(time.strftime('%X %x %Z')))
else:
  print('Expected a .aut file, but got %s' %(name))
