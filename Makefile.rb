
rule "csem.zip" do
    shell "zip csem.zip mip/* i386/*"
end

rule "copy", :d => "csem.zip" do
    shell "scp csem.zip bjr5e@power1.cs.virginia.edu:/af3/bjr5e/cs671/csem/"
end
