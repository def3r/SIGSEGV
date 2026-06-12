using LLVM

context!(Context()) do
  mod = parse(LLVM.Module, String(read("after_mem2reg.ll")))
  for func in functions(mod)
    println("Function: ", name(func))
    for bb in blocks(func)
      println(" Block: ", name(bb))
      for inst in instructions(bb)
        println("    ", inst)
      end
    end
  end
end
