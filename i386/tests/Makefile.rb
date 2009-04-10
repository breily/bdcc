
tests = ["arith", "for"]

rule "all" do
    tests.each do |t|
        shell "../csem -codegen < #{t}_tests.c > #{t}_tests.s", :silent
        shell "gcc -m32 -o #{t}_tests #{t}_tests.s", :silent
        shell "./#{t}_tests", :silent
    end
end

clean "*.s", "*.o", 
      "arith_tests",
      "for_tests",
      "if_tests"
