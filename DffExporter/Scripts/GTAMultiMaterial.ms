plugin Material GTA_MultiMat
name: "GTA Multi/Sub-Object"
classID: #(0x744721ba, 0x31ec52e0)
extends: MultiMaterial replaceUI: false version: 1
(
	parameters main rollout: params
	(
		--OverrideDialog type: #boolean default: true UI: ui_Override --0
		--Modified again 7th March 2015. Remember to add this file to source control.
		faceType type: #integer default: 1 UI: ui_faceType -- 0
		RenderPipe type: #integer default: 1 UI: ui_RenderPipe --1
		meshExt	type: #boolean default: true UI: ui_meshExt --2
		skyMipMap type: #boolean default: false UI: ui_skyMipMap --3
		morphPLG type: #boolean default: false UI: ui_morphPLG --4
		nvc type: #boolean default: false UI: ui_nvc --5
		UVmap1 type: #boolean default: true UI: ui_UVmap1 --6
		UVmap2 type: #boolean default: false UI: ui_UVmap2 --7
		vcolors type: #boolean default: false UI: ui_vcolors --8
		normals type: #boolean default: true UI: ui_normals --9
		dynlight type: #boolean default: true UI: ui_dynlight --10
		mmc type: #boolean default: true UI: ui_mmc --11
		nvcHeader type:#string default:"0e872b00" UI: ui_nvcHeader --12
		amb type: #float default: 50 UI: ui_Amb --13
		diff type: #float default: 50 UI: ui_Diff --14
		spec type: #float default: 50 UI: ui_Spec --15
	)
	
		
	rollout params "Geometry options"
	(
		--checkbox ui_Override "Override corresponding dialog options" align: #center
		label ui_faceTypeLabel "Face type" align: #left offset: [0,3] across: 2
		dropDownList ui_faceType width: 140 offset: [-70,0] items: #("Triangle list", "Triangle Strip (NVDIA)", "Triangle Strip (Zappy)")
		label ui_RenderPipeLabel "Render pipeline" align: #left offset: [0,3] across: 2
		dropDownList ui_RenderPipe width: 140 offset: [-70,0] items: #("None", "Reflective building", "Night vertex colors", "Vehicle")
		checkBox ui_meshExt "Write Mesh Extension" across: 2
		checkBox ui_skyMipMap "Write Sky Mip Map"
		checkBox ui_morphPLG "Write Morph PLG" across: 2
		checkbox ui_nvc "Night vertex colors"
		checkbox ui_uvmap1 "UV map 1" across: 2
		checkbox ui_uvmap2 "UV map 2"
		checkbox ui_vcolors "vertex colors" across: 2
		checkbox ui_normals "normals"
		checkbox ui_dynlight "dynamic lighting" across:2
		checkbox ui_mmc "modulate material color"
		group "Night Vertex Colors"
		(
			edittext ui_nvcHeader "Header"
		)
		group "Version Specific"
		(
			spinner ui_Amb "Ambient" type: #float range: [0,100,20] across: 3
			spinner ui_Diff "Diffuse" type: #float range: [0,100,20]
			spinner ui_Spec "Specular" type: #float range: [0,100,20]
		)
	)
	
	on create do (
		this.name = delegate.name
		delegate.numsubs= 1
	)
	
	on load do (
		this.name= delegate.name
	)

)