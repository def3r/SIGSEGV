source_filename = "addition.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [15 x i8] c"%d + %d = %d; \00", align 1
@stdout = external global ptr, align 8
@qkey = private unnamed_addr constant [10 x i8] c"add_71_11\00", align 1
@qdecoder = private unnamed_addr constant [4 x i8] c"add\00", align 1
@qkey.1 = private unnamed_addr constant [10 x i8] c"add_10_20\00", align 1
@qdecoder.2 = private unnamed_addr constant [4 x i8] c"add\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @sum(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %0, %1
  ret i32 %3
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = call i32 @sum(i32 noundef 71, i32 noundef 11)
  %2 = call i32 @quantum_execute(ptr @qkey, ptr @qdecoder)
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef 71, i32 noundef 11, i32 noundef %2)
  %4 = load ptr, ptr @stdout, align 8
  %5 = call i32 @fflush(ptr noundef %4)
  %6 = call i32 @quantum_execute(ptr @qkey.1, ptr @qdecoder.2)
  %7 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef 10, i32 noundef 20, i32 noundef %6)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #1

declare i32 @fflush(ptr noundef) #1

declare i32 @quantum_execute(ptr, ptr)

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 15.0.7 (https://github.com/llvm/llvm-project 8dfdcc7b7bf66834a761bd8de445840ef68e4d1a)"}
