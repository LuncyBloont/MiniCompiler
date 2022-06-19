#pragma once
#ifndef GRAMMAR_H
#define GRAMMAR_H
const char Grammar_0[] = "\
<Program> ==> <SubProgram>									CSDO *MKMEM\n\
\n\
# <SubProgram> ==> [<ConstVar>][<NormalVar>][<FunctionVar>]<RunableCode>\n\
<SubProgram> ==> <ConstVar> <A>|<A>					CSDO TABLE $POPTABLE|TABLE $POPTABLE\n\
<A> ==> <NormalVar> <B>|<B>						CSDO\n\
<B> ==> <FunctionVar> <RunableCode>|<RunableCode>					CSDO\n\
\n\
# <ConstVar> ==> CONST<ConstDefine>{,<ConstDefine>};\n\
<ConstVar> ==> const <ConstDefine> <C> ;           	CSDO\n\
<C> ==> __0__|, <ConstDefine> <C>                     	CSDO\n\
\n\
<ConstDefine> ==> <ID> = <UInt>					CSDO TESTNDEF(0) DEFC(0,2)\n\
\n\
# <UInt> ==> <SingleNumber>{<SingleNumber>}\n\
<UInt> ==> <SingleNumber> <D>                        	CSDO \n\
<D> ==> __0__|<SingleNumber> <D>								CSDO\n\
\n\
# <NormalVar> ==> VAR<ID>{,<ID>};			\n\
<NormalVar> ==> var <ID> <E> ;					CSDO TESTNDEF(1) DEF(1)\n\
<E> ==> __0__|, <ID> <E>							CSDO |TESTNDEF(1) DEF(1)\n\
\n\
# <ID> ==> <Alpha>{<Alpha>|<SingleNumber>}\n\
<ID> ==> <Alpha> <F>								CSDO\n\
<F> ==> __0__|<Alpha> <F>|<SingleNumber> <F>					CSDO\n\
\n\
# <FunctionVar> ==> <FunctionHead><SubProgram>;{<FunctionVar>}\n\
<FunctionVar> ==> <FunctionHead> <SubProgram> ; <G>				CSDO $RESPC\n\
<G> ==> __0__|<FunctionHead> <SubProgram> ; <G>					CSDO |$RESPC\n\
\n\
<FunctionHead> ==> procedure <ID> ;						CSDO TESTNDEF(1) DEFF(1)\n\
<RunableCode> ==> <Set>|<Condition>|<WhileLoop>|<Call>|<Read>|<Write>|<MultiCode>|__0__     CSDO\n\
<Set> ==> <ID> := <Expression>						CSDO TESTDEF(0) TESTM(0) *RG $SET(0) $POPRG\n\
\n\
# <MultiCode> ==> begin<RunableCode>{ ;<RunableCode>}end\n\
<MultiCode> ==> begin <RunableCode> <H> end						CSDO TABLE\n\
<H> ==> __0__|; <RunableCode> <H>								CSDO POPTABLE|\n\
\n\
<ConditionExpression> ==> <Expression> <RelationalOps> <Expression>|odd <Expression>     CSDO *RG $CMP $POPRG $POPRG|*RG $ODD $POPRG\n\
\n\
# <Expression> ==> [+|-]<Item>{<AddOrSub><Item>}\n\
<Expression> ==> + <Item> <I>|- <Item> <I>|<Item> <I>				CSDO $WTOPTOBTOP|$INVRG $WTOPTOBTOP|$WTOPTOBTOP\n\
<I> ==> __0__|<AddOrSub> <Item> <I>						CSDO |*RG $OP(0) $POPRG\n\
\n\
# <Item> ==> <Factor>{<MulOrDiv><Factor>}\n\
<Item> ==> <Factor> <J>										CSDO $POPRG\n\
<J> ==> __0__|<MulOrDiv> <Factor> <J>						CSDO |*RG $OP(0) $POPRG\n\
\n\
<Factor> ==> <ID>|<UInt>|( <Expression> )				CSDO TESTDEF(0) *WRG(0)|*WRGI(0)|*RG\n\
<AddOrSub> ==> +|-										CSDO\n\
<MulOrDiv> ==> *|/										CSDO\n\
<RelationalOps> ==> =|#|<|<=|>|>=							CSDO\n\
<Condition> ==> if <ConditionExpression> then <RunableCode>						CSDO *RG *CODEADDR *JP $WADRRTOCODEADDR $POPCODEADRR\n\
<Call> ==> call <ID>							CSDO TESTDEF(1) *SAVEPC *CALL(1)\n\
<WhileLoop> ==> while <ConditionExpression> do <RunableCode>					CSDO *RG *J *CODEADDR *ADRR $WADRRTOCODEADDR $POPCODEADRR $JP \n\
\n\
# <Read> ==> read(<ID>{,<ID>})\n\
<Read> ==> read ( <ID> <K> )						CSDO TESTDEF(2) *READ(2)\n\
<K> ==> __0__|, <ID> <K>							CSDO |TESTNDEF(1) *READ(1)\n\
\n\
# <Write> ==> write(<ID>{,<ID>})\n\
<Write> ==> write ( <ID> <L> )						CSDO TESTDEF(2) *WRITE(2)\n\
<L> ==> __0__|, <ID> <L>							CSDO |TESTNDEF(1) *WRITE(1)\n\
\n\
<Alpha> ==> a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z    CSDO\n\
<SingleNumber> ==> 0|1|2|3|4|5|6|7|8|9							CSDO\n\
\n\
\n\
";
#endif // GRAMMAR_H