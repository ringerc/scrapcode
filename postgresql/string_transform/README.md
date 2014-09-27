A quick answer to Stack Overflow question http://stackoverflow.com/q/26072947/398670

Demoes how to transform strings with nested brackets, substituting brackets for braces
only for a certain level of nesting, e.g.

    [0,0,0,[12,2],0,0,[12,[1,2,3]],12,0,[12,2,[2]],12,0,12,0,0]

into

    {0,0,0,{12,2},0,0,{12,[1,2,3]},12,0,{12,2,[2]},12,0,12,0,0}
