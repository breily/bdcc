
tests = ["arith", "for", "shift", "negate", "comp"]

rule "all" do
    tests.each do |t|
        shell "../csem -codegen < #{t}_tests.c > #{t}_tests.s", :silent
        shell "gcc -m32 -o #{t}_tests.out #{t}_tests.s", :silent
        shell "./#{t}_tests.out", :silent
    end
end

clean "*.s", "*.o", "*.out"
